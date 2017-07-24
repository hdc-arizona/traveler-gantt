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

Trace::Trace(int nt, int np)
    : name(""),
      fullpath(""),
      num_entities(nt),
      num_pes(np),
      units(-9),
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
                       unsigned long long entity_stop,
                       unsigned long long min_span)
{
    json jo;
    if (start > stop || start > max_time || stop < min_time)
    {
        return jo;
    }

    jo["path"] = fullpath;
    jo["mintime"] = min_time;
    jo["maxtime"] = max_time;
    jo["starttime"] = start;
    jo["stoptime"] = stop;


    std::vector<std::vector<std::vector<Event *> *> *> * event_slice
        = new std::vector<std::vector<std::vector<Event *> *> *>();
    event_slice->push_back(new std::vector<std::vector<Event *> *>());
    for (unsigned long long entity = entity_start; entity < entity_stop; entity++)
    {
        event_slice->back()->push_back(new std::vector<Event *>());
    }

    for (unsigned long long entity = entity_start; entity < entity_stop; entity++)
    {
        for (std::vector<Event *>::iterator root = roots->at(entity)->begin();
             root != roots->at(entity)->end(); ++root)
        {
            timeEventToJSON(*root, 0, start, stop, entity_start, entity_stop,
                            min_span, event_slice);
        }
    }


    // Should autoconvert from std::vector to json
    //jo["events"] = event_slice;

    std::vector<std::vector<json> > slices = std::vector<std::vector<json> >();
    // Per depth
    for (std::vector<std::vector<std::vector<Event *> *> *>::iterator es = event_slice->begin();
         es != event_slice->end(); ++es)
    {
        std::vector<json> atdepth = std::vector<json>();

        // Per entity 
        for (std::vector<std::vector<Event *> *>::iterator ent = (*es)->begin();
            ent != (*es)->end(); ++ent)
        {
            atdepth->push_back(*ent);
        }
        slices->push_back(atdepth);
    }
    jo["events"] = slices;


    for (std::vector<std::vector<std::vector<Event *> *> *>::iterator es = event_slice->begin();
         es != event_slice->end(); ++es)
    {
        for (std::vector<std::vector<Event *> *>::iterator ent = (*es)->begin();
            ent != (*es)->end(); ++ent)
        {
            delete *ent;
        }
        delete *es;
    }
    delete event_slice;

    return jo;
}

void Trace::timeEventToJSON(Event * evt, int depth, unsigned long long start,
    unsigned long long stop, unsigned long long entity_start, unsigned long long entity_stop,
    unsigned long long min_span,
    std::vector<std::vector<std::vector<Event *> *> *> * slice)
{
    // Make sure the event is in range
    if (!(evt->enter < stop && evt->exit > start))
    {
        return;
    }

    // Make sure we have a vector at this depth
    if (depth >= slice->size())
    {
        slice->push_back(new std::vector<std::vector<Event *> *>());
        for (unsigned long long entity = entity_start; entity < entity_stop; entity++)
        {
            slice->back()->push_back(new std::vector<Event *>());
        }
    }

    // Add the event
    if ((evt->exit - evt->enter) > min_span)
    {
        slice->at(depth)->at(evt->entity - entity_start)->push_back(evt);

        // Add children
        for (std::vector<Event *>::iterator child = evt->callees->begin();
            child != evt->callees->end(); ++child)
        {
            timeEventToJSON(*child, depth + 1, start, stop, entity_start,
                            entity_stop, min_span, slice);
        }
    }

}

json Trace::initJSON()
{
    return timeToJSON(0, 90000, 0, roots->size() - 1);
}
