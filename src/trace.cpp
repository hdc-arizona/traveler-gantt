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
                       unsigned long width)
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

    std::vector<json> event_slice = std::vector<json>();
    std::vector<json> msg_slice = std::vector<json>();
    std::vector<json> collective_slice = std::vector<json>();
    std::vector<std::vector<json> >  parent_slice
        = std::vector<std::vector<json> >();
    parent_slice.push_back(std::vector<json>());
    std::map<std::string, std::string> function_names = std::map<std::string, std::string>();

    unsigned long long entity_stop = entity_start + entities;
    for (unsigned long long entity = entity_start; entity < entity_stop; entity++)
    {
        for (std::vector<Event *>::iterator root = roots->at(entity)->begin();
             root != roots->at(entity)->end(); ++root)
        {
            timeEventToJSON(*root, 0, start, stop, entity_start, entities,
                            a_pixel, event_slice, msg_slice, collective_slice,
                            parent_slice, function_names);
        }
    }


    // Should autoconvert from std::vector and std::map to json
    jo["events"] = event_slice;
    jo["parent_events"] = parent_slice;
    jo["messages"] = msg_slice;
    jo["collectives"] = collective_slice;
    jo["functions"] = function_names;

    return jo;
}

void Trace::timeEventToJSON(Event * evt, int depth, unsigned long long start,
    unsigned long long stop, unsigned long long entity_start, unsigned long long entities,
    unsigned long long min_span,
    std::vector<json>& slice,
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
        json jevt(evt);
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
            slice.push_back(jevt);
            std::vector<Message *> * messages = cevt->getMessages();
            if (messages != NULL) {
                for (std::vector<Message *>::iterator msg = messages->begin();
                    msg != messages->end(); ++msg)
                {
                    if ((*msg)->recvtime > stop || !(evt == (*msg)->sender)) 
                    //if ((*msg)->sendtime < start || !(evt == (*msg)->receiver)) 
                    {
                        std::cout << "Attempting to write a message" << std::endl;
                        json jmsg(*msg);
                        msg_slice.push_back(jmsg);
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

            parent_slice.at(depth).push_back(jevt);
        }

        // Add children
        for (std::vector<Event *>::iterator child = evt->callees->begin();
            child != evt->callees->end(); ++child)
        {
            if ((*child)->enter > stop)
                break;
            timeEventToJSON(*child, depth + 1, start, stop, entity_start,
                            entities, min_span, slice, msg_slice, 
                            collective_slice, parent_slice, 
                            function_names);
        }
    }

}

json Trace::timeOverview(unsigned long width)
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

json Trace::initJSON(unsigned long width)
{
    unsigned long long a_pixel = (last_finalize - last_init) / width;
    unsigned long long span = 1000000;
    if (a_pixel * 5 > span)
    {
        span = a_pixel * 5;
    }
    json jo = timeToJSON(last_init, span + last_init, 0, roots->size(), width);
    jo["overview"] = timeOverview(width);

    return jo;
}
