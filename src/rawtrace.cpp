#include "rawtrace.h"

#include "primaryentitygroup.h"
#include "entity.h"
#include "eventrecord.h"
#include "commrecord.h"
#include "entitygroup.h"
#include "otfcollective.h"
#include "collectiverecord.h"
#include "function.h"
#include "counter.h"
#include "counterrecord.h"
#include <stdint.h>


RawTrace::RawTrace(int nt, int np)
    : primaries(NULL),
      processingElements(NULL),
      functionGroups(NULL),
      functions(NULL),
      events(NULL),
      messages(NULL),
      messages_r(NULL),
      entitygroups(NULL),
      collective_definitions(NULL),
      counters(NULL),
      counter_records(NULL),
      collectives(NULL),
      collectiveMap(NULL),
      collectiveBits(NULL),
      num_entities(nt),
      num_pes(np),
      second_magnitude(1),
      metric_names(NULL),
      metric_units(NULL)
{

}

// Note we do not delete the function/functionGroup map because
// we know that will get passed to the processed trace
RawTrace::~RawTrace()
{
    for (std::vector<std::vector<EventRecord *> *>::iterator eitr = events->begin();
         eitr != events->end(); ++eitr)
    {
        for (std::vector<EventRecord *>::iterator itr = (*eitr)->begin();
             itr != (*eitr)->end(); ++itr)
        {
            delete *itr;
            *itr = NULL;
        }
        delete *eitr;
        *eitr = NULL;
    }
    delete events;

    for (std::vector<std::vector<CommRecord *> *>::iterator eitr = messages->begin();
         eitr != messages->end(); ++eitr)
    {
        for (std::vector<CommRecord *>::iterator itr = (*eitr)->begin();
             itr != (*eitr)->end(); ++itr)
        {
            delete *itr;
            *itr = NULL;
        }
        delete *eitr;
        *eitr = NULL;
    }
    delete messages;
    delete messages_r;

    for (std::vector<std::vector<CollectiveBit *> *>::iterator eitr = collectiveBits->begin();
         eitr != collectiveBits->end(); ++eitr)
    {
        for (std::vector<CollectiveBit *>::iterator itr = (*eitr)->begin();
             itr != (*eitr)->end(); ++itr)
        {
            delete *itr;
            *itr = NULL;
        }
        delete *eitr;
        *eitr = NULL;
    }
    delete collectiveBits;

    if (metric_names)
        delete metric_names;
    if (metric_units)
        delete metric_units;
}
