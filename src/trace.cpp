#include "trace.h"

#include <iostream>
#include <fstream>
#include <cmath>
#include <climits>
#include <cfloat>
#include <algorithm>

#include "entity.h"
#include "event.h"
#include "commevent.h"
#include "p2pevent.h"
#include "collectiveevent.h"
#include "function.h"
#include "entitygroup.h"
#include "otfcollective.h"
#include "ravelutils.h"
#include "primaryentitygroup.h"
#include "metrics.h"
#include "message.h"

Trace::Trace(int nt, int np)
    : name(""),
      fullpath(""),
      num_entities(nt),
      num_pes(np),
      units(-9),
      max_depth(0),
      totalTime(0), // for paper timing
      metrics(new std::vector<std::string>()),
      metric_units(new std::map<std::string, std::string>()),
      functionGroups(new std::map<int, std::string>()),
      functions(new std::map<int, Function *>()),
      primaries(NULL),
      processingElements(NULL),
      entitygroups(NULL),
      collective_definitions(NULL),
      collectives(NULL),
      collectiveMap(NULL),
      events(new std::vector<std::vector<Event *> *>(std::max(nt, np))),
      roots(new std::vector<std::vector<Event *> *>(std::max(nt, np))),
      guidMap(new std::map<uint64_t, std::vector<unsigned long long> *>()),
      mpi_group(-1),
      max_time(0),
      min_time(ULLONG_MAX),
      isProcessed(false)
{
    for (int i = 0; i < std::max(nt, np); i++) {
        (*events)[i] = new std::vector<Event *>();
    }

    for (int i = 0; i < std::max(nt, np); i++)
    {
        (*roots)[i] = new std::vector<Event *>();
    }
}

Trace::~Trace()
{
    delete metrics;
    delete metric_units;
    delete functionGroups;

    for (std::map<int, Function *>::iterator itr = functions->begin();
         itr != functions->end(); ++itr)
    {
        delete (itr->second);
        itr->second = NULL;
    }
    delete functions;

    for (std::vector<std::vector<Event *> *>::iterator eitr = events->begin();
         eitr != events->end(); ++eitr)
    {
        for (std::vector<Event *>::iterator itr = (*eitr)->begin();
             itr != (*eitr)->end(); ++itr)
        {
            delete *itr;
            *itr = NULL;
        }
        delete *eitr;
        *eitr = NULL;
    }
    delete events;

    // Don't need to delete Events, they were deleted above
    for (std::vector<std::vector<Event *> *>::iterator eitr = roots->begin();
         eitr != roots->end(); ++eitr)
    {
        delete *eitr;
        *eitr = NULL;
    }
    delete roots;

    for (std::map<int, EntityGroup *>::iterator comm = entitygroups->begin();
         comm != entitygroups->end(); ++comm)
    {
        delete comm->second;
        comm->second = NULL;
    }
    delete entitygroups;

    for (std::map<int, OTFCollective *>::iterator cdef = collective_definitions->begin();
         cdef != collective_definitions->end(); ++cdef)
    {
        delete cdef->second;
        cdef->second = NULL;
    }
    delete collective_definitions;

    for (std::map<unsigned long long, CollectiveRecord *>::iterator coll = collectives->begin();
         coll != collectives->end(); ++coll)
    {
        delete coll->second;
        coll->second = NULL;
    }
    delete collectives;

    delete collectiveMap;

    for (std::map<int, PrimaryEntityGroup *>::iterator primary = primaries->begin();
         primary != primaries->end(); ++primary)
    {
        for (std::vector<Entity *>::iterator entity = primary->second->entities->begin();
             entity != primary->second->entities->end(); ++entity)
        {
            delete *entity;
            *entity = NULL;
        }
        delete primary->second;
    }
    delete primaries;
    
    for (std::map<uint64_t, std::vector<unsigned long long> *>::iterator itr 
        = guidMap->begin(); itr != guidMap->end(); ++itr)
    {
        delete (itr->second);
        itr->second = NULL;
    }
    delete guidMap;
}

void Trace::preprocess()
{
    isProcessed = true;
}

// Find the smallest event in a timeline that contains the given time
Event * Trace::findEvent(int entity, unsigned long long time)
{
    if (entity < 0 || entity >= roots->size())
        return NULL;

    Event * found = NULL;
    for (std::vector<Event *>::iterator root = roots->at(entity)->begin();
         root != roots->at(entity)->end(); ++root)
    {
        found = (*root)->findChild(time);
        if (found)
            return found;
    }

    return found;
}

json Trace::timeToJSON(unsigned long long start, unsigned long long stop, 
                       unsigned long long entity_start,
                       unsigned long long entities,
                       unsigned long width, 
                       unsigned long long taskid,
                       unsigned long long task_time,
                       unsigned long traceback_state,
                       unsigned long long hover,
                       bool logging)
{
    json jo;

    jo["path"] = fullpath;
    jo["mintime"] = last_init;
    jo["maxtime"] = last_finalize + 10; // So the finalize has some length
    jo["starttime"] = start;
    jo["stoptime"] = stop;
    jo["max_depth"] = max_depth;
    jo["entities"] = entities;
    jo["entity_start"] = entity_start;
    if (start > stop || start > max_time || stop < min_time)
    {
        jo["error"] = "Incorrect time range.";
        return jo;
    }

    // Determine min_span of half a pixel in width
    unsigned long long a_pixel = (stop - start) / width / 2;

    std::set<uint64_t> event_set = std::set<uint64_t>();
    std::vector<json> event_slice = std::vector<json>();
    std::vector<json> msg_slice = std::vector<json>();
    std::vector<json> collective_slice = std::vector<json>();
    std::vector<std::vector<json> >  parent_slice
        = std::vector<std::vector<json> >();
    parent_slice.push_back(std::vector<json>());
    std::map<std::string, std::string> function_names = std::map<std::string, std::string>();

    unsigned long long entity_stop = entity_start + entities;
   
    // Trace back events
    if (taskid != 0)
    {
        bool full_traceback = false;
        if (traceback_state == traceback_full) {
            full_traceback = true;
        }
        for (unsigned long long entity = entity_start; entity < entity_stop; entity++)
        {
            for (std::vector<Event *>::iterator root = roots->at(entity)->begin();
                 root != roots->at(entity)->end(); ++root)
            {
                eventTraceBackJSON(*root, start, stop, entity_start, entities,
                                   a_pixel, taskid, task_time, full_traceback, msg_slice, 
                                   event_slice, event_set, function_names, logging);
            }
        }
    }

    // All events
    for (unsigned long long entity = entity_start; entity < entity_stop; entity++)
    {
        for (std::vector<Event *>::iterator root = roots->at(entity)->begin();
             root != roots->at(entity)->end(); ++root)
        {
            timeEventToJSON(*root, 0, start, stop, entity_start, entities,
                            a_pixel, taskid, event_slice, event_set, 
                            msg_slice, collective_slice,
                            parent_slice, function_names);
        }
    }


    // Should autoconvert from std::vector and std::map to json
    jo["events"] = event_slice;
    jo["parent_events"] = parent_slice;
    jo["messages"] = msg_slice;
    jo["collectives"] = collective_slice;
    jo["functions"] = function_names;

    std::vector<std::string> hover_strings = std::vector<std::string>();
    if (hover)
    {
        std::vector<unsigned long long> * hover_ids = guidMap->at(hover);
        for (std::vector<unsigned long long>::iterator itr = hover_ids->begin();
            itr != hover_ids->end(); ++itr)
        {
            hover_strings.push_back(std::to_string(*itr));
        }
    }
    jo["hover_ids"] = hover_strings;

    return jo;
}

void Trace::msgTraceBackJSON(CommEvent * evt, int depth, bool sibling, bool full_traceback, Message * last, 
    unsigned long long start, unsigned long long stop, unsigned long long entity_start, unsigned long long entities,
    unsigned long long min_span, std::vector<json>& msg_slice, std::vector<json>& evt_slice, 
    std::set<uint64_t>& evt_set, std::map<std::string, std::string>& function_names, bool logging)
{
    if (((evt_set.find(evt->id) == evt_set.end()) && ((evt->exit - evt->enter) > min_span))
        && (!sibling || full_traceback))
    {
        json jevt(evt);
        jevt["depth"] = depth;
        jevt["sibling"] = sibling;
        evt_slice.push_back(jevt);
        evt_set.insert(evt->id);

        function_names.insert(std::pair<std::string, std::string>(std::to_string(evt->function), 
                                                                  functions->at(evt->function)->name));
    }

    std::vector<Message *> * messages = evt->getMessages();
    if (messages != NULL) {
        for (std::vector<Message *>::iterator msg = messages->begin();
            msg != messages->end(); ++msg)
        {
            if (logging) 
            {
                std::cout << "On message " << (*msg)->sendtime << " to " << (*msg)->recvtime;
                std::cout << " received by " << (*msg)->receiver->getGUID() << std::endl;
            }
            // Note we only add if the event is the receive and if the
            // message is actually in the time frame we're looking for
            if (logging && ((*msg)->sender == NULL || (*msg)->receiver == NULL))
            {
                std::cout << "     Null message." << std::endl;
            }

            // If we're the receiver, write out the traceback message
            if ((*msg)->recvtime > start 
                && (*msg)->sender != NULL && (*msg)->receiver != NULL
                && (*msg)->receiver == evt)
            {
                json jmsg(*msg);
                jmsg["depth"] = depth;
                jmsg["sibling"] = false;
                msg_slice.push_back(jmsg);
            }

            // If we're the sender and on the main path (!sibling)
            // AND we're doing a full traceback:
            //   add forward messages as well
            else if (!sibling && full_traceback
                     && (*msg)->sender == evt && (*msg) != last
                     && (*msg)->sender != NULL && (*msg)->receiver != NULL
                     && (*msg)->recvtime > start)
            {
                json jmsg(*msg);
                jmsg["depth"] = depth - 1;
                jmsg["sibling"] = true;
                msg_slice.push_back(jmsg);
                CommEvent * rcv = (*msg)->receiver;
                if ((evt_set.find(rcv->id) == evt_set.end()) && ((rcv->exit - rcv->enter) > min_span))
                {
                    json revt(rcv);
                    revt["depth"] = depth - 1;
                    revt["sibling"] = true;
                    evt_slice.push_back(revt);
                    evt_set.insert(rcv->id);

                    function_names.insert(std::pair<std::string, std::string>(std::to_string(rcv->function), 
                                                                              functions->at(rcv->function)->name));
                }
            }


            // If we're not on a sibling line and we're still in the time
            // window, continue tracing back.
            if (!sibling && (*msg)->receiver == evt && evt->exit > start && (*msg)->sender) 
            {
                if (logging) 
                    std::cout << "     Tracing back to sender " << (*msg)->sender->getGUID() << std::endl;
                msgTraceBackJSON((*msg)->sender, depth + 1, false, full_traceback, *msg, start, stop,
                                 entity_start, entities, min_span, 
                                 msg_slice, evt_slice, evt_set, function_names, logging);
            }

        }
    }
}

void Trace::eventTraceBackJSON(Event * evt, unsigned long long start,
    unsigned long long stop, unsigned long long entity_start, unsigned long long entities,
    unsigned long long min_span, unsigned long long taskid, unsigned long long task_time,
    bool full_traceback,
    std::vector<json>& msg_slice, std::vector<json>& evt_slice, std::set<uint64_t>& evt_set,
    std::map<std::string, std::string>& function_names, bool logging)
{
    if (logging && evt->getGUID() == taskid)
    {
        std::cout << "GUID match at: " << task_time << " in " << evt->enter << " and " << evt->exit << std::endl;
        std::cout << "      Task ID is " << taskid << " and event GUID is " << evt->getGUID() << std::endl;
    }

    // Make sure the event is in range
    if (!(evt->enter <= task_time && evt->exit >= task_time))
    {
        return;
    }

    // Search for focus event
    if (evt->getGUID() == taskid)
    {
        if (logging)
            std::cout << ">>>Event Found!<<<" << std::endl;
        if (evt->isCommEvent()) 
        {
            if (logging)
                std::cout << "   is Comm, starting traceback" << std::endl;

            // Traceback from that event
            msgTraceBackJSON(static_cast<CommEvent *>(evt), 0, false, full_traceback, NULL, start, stop,
                             entity_start, entities, min_span, msg_slice, evt_slice, evt_set,
                             function_names, logging);
        } 
    }
    else
    {
        // Search children
        for (std::vector<Event *>::iterator child = evt->callees->begin();
            child != evt->callees->end(); ++child)
        {
            if ((*child)->enter > stop)
            {
                if (logging)
                    std::cout << "Child enters after stop, ignore." << std::endl; 
                continue;
            }
            eventTraceBackJSON(*child, start, stop, entity_start,
                               entities, min_span, taskid, task_time, full_traceback,
                               msg_slice, evt_slice, evt_set, function_names, logging); 
        }
    }
}

void Trace::timeEventToJSON(Event * evt, int depth, unsigned long long start,
    unsigned long long stop, unsigned long long entity_start, unsigned long long entities,
    unsigned long long min_span, unsigned long long taskid, 
    std::vector<json>& slice,
    std::set<uint64_t>& slice_set, 
    std::vector<json>& msg_slice,
    std::vector<json>& collective_slice,
    std::vector<std::vector<json> >& parent_slice,
    std::map<std::string, std::string>& function_names)
{
    // Make sure the event is in range
    if (!(evt->enter < stop && evt->exit > start))
    {
        return;
    }

    // Add the event
    if ((evt->exit - evt->enter) > min_span)
    {
        function_names.insert(std::pair<std::string, std::string>(std::to_string(evt->function), 
                                                                  functions->at(evt->function)->name));
        if (evt->isCommEvent()) 
        {
            CommEvent * cevt = static_cast<CommEvent *>(evt);
            json jevt(cevt);
            //if (cevt->hasMetric(metric)) 
            //{
            //    jevt["metrics"] = { cevt->getMetric(metric), cevt->getMetric(metric, true) };
           // }
           // if (cevt->isP2P())
           // {
           //     P2PEvent * pevt = static_cast<P2PEvent *>(cevt);
           //     if (pevt->subevents != NULL)
           //     {
           //         jevt["coalesced"] = 1;
           //     }
           // }
            if (slice_set.find(evt->id) == slice_set.end())
                slice.push_back(jevt);
            if (taskid == 0) 
            {
                std::vector<Message *> * messages = cevt->getMessages();
                if (messages != NULL) {
                    for (std::vector<Message *>::iterator msg = messages->begin();
                        msg != messages->end(); ++msg)
                    {
                        if (((*msg)->recvtime > stop || !(evt == (*msg)->sender)) 
                            && (*msg)->sender != NULL && (*msg)->receiver != NULL)
                        //if ((*msg)->sendtime < start || !(evt == (*msg)->receiver)) 
                        {
                            //std::cout << "Attempting to write a message" << std::endl;
                            json jmsg(*msg);
                            jmsg["depth"] = 0;
                            jmsg["sibling"] = false;
                            msg_slice.push_back(jmsg);
                        }
                    }
                }
            }
        } 
        else 
        {
            // Make sure we have a vector at this depth
            if (depth >= parent_slice.size())
            {
                parent_slice.push_back(std::vector<json>());
            }

            json jevt(evt);
            parent_slice.at(depth).push_back(jevt);
        }

        // Add children
        for (std::vector<Event *>::iterator child = evt->callees->begin();
            child != evt->callees->end(); ++child)
        {
            if ((*child)->enter > stop)
                break;
            timeEventToJSON(*child, depth + 1, start, stop, entity_start,
                            entities, min_span, taskid, slice, slice_set, msg_slice, 
                            collective_slice, parent_slice, 
                            function_names);
        }
    }

}

json Trace::timeOverview(unsigned long width, bool logging)
{
    unsigned long long a_pixel = (last_finalize - last_init) / width;
    std::vector<unsigned long long> pixels = std::vector<unsigned long long>();
    //std::cout << "width is " << width << " and start " << last_init << " and stop " << last_finalize << std::endl;
    for (unsigned long i = 0; i <= width; i++)
    {
        pixels.push_back(0);
    }
    for (unsigned long long entity = 0; entity < events->size(); entity++)
    {
        for (std::vector<Event *>::iterator evt = events->at(entity)->begin();
             evt != events->at(entity)->end(); ++evt)
        {
            if (!(*evt)->isCommEvent())
            {
                continue;
            }
            unsigned long pixel_start = 0;
            unsigned long pixel_end = width;
            if ((*evt)->enter > last_init)
            {
                pixel_start = ((*evt)->enter - last_init) / a_pixel;
            }
            if ((*evt)->exit < last_finalize)
            {
                pixel_end = ((*evt)->exit - last_init) / a_pixel + 1;
            }
            for (unsigned long i = pixel_start; i < pixel_end; i++)
            {
                pixels[i] += 1;
            }
        }
    }
    json jo(pixels);
    return jo;
}

json Trace::initJSON(unsigned long width, bool logging)
{
    unsigned long long a_pixel = (last_finalize - last_init) / width;
    unsigned long long span = 1000000;
    if (a_pixel * 5 > span)
    {
        span = a_pixel * 5;
    }
    json jo = timeToJSON(last_init, span + last_init, 0, roots->size(), width, 0, 0, 0, 0, logging);
    jo["overview"] = timeOverview(width, logging);

    return jo;
}
