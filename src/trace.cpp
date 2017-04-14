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

    for (std::map<int, Function *>::Iterator itr = functions->begin();
         itr != functions->end(); ++itr)
    {
        delete (itr.value());
        itr.value() = NULL;
    }
    delete functions;

    for (std::vector<std::vector<Event *> *>::Iterator eitr = events->begin();
         eitr != events->end(); ++eitr)
    {
        for (std::vector<Event *>::Iterator itr = (*eitr)->begin();
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
        delete *comm;
        *comm = NULL;
    }
    delete entitygroups;

    for (std::map<int, OTFCollective *>::iterator cdef = collective_definitions->begin();
         cdef != collective_definitions->end(); ++cdef)
    {
        delete *cdef;
        *cdef = NULL;
    }
    delete collective_definitions;

    for (std::map<unsigned long long, CollectiveRecord *>::iterator coll = collectives->begin();
         coll != collectives->end(); ++coll)
    {
        delete *coll;
        *coll = NULL;
    }
    delete collectives;

    delete collectiveMap;

    for (std::map<int, PrimaryEntityGroup *>::iterator primary = primaries->begin();
         primary != primaries->end(); ++primary)
    {
        for (std::vector<Entity *>::iterator entity = primary.value()->entities->begin();
             entity != primary.value()->entities->end(); ++entity)
        {
            delete *entity;
            *entity = NULL;
        }
        delete primary.value();
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
