#include "otfconverter.h"
#include <cmath>
#include <ctime>
#include <climits>
#include <iostream>
#include <algorithm>

#include "ravelutils.h"

#ifdef OTF1LIB
#include "otfimporter.h"
#endif

#include "otf2importer.h"
#include "rawtrace.h"
#include "trace.h"
#include "counter.h"
#include "function.h"
#include "collectiverecord.h"
#include "eventrecord.h"
#include "commrecord.h"
#include "counterrecord.h"
#include "event.h"
#include "commevent.h"
#include "p2pevent.h"
#include "message.h"
#include "collectiveevent.h"
#include "primaryentitygroup.h"
#include "metrics.h"


const std::string OTFConverter::collectives_string
    = std::string("MPI_BarrierMPI_BcastMPI_ReduceMPI_GatherMPI_Scatter")
      + std::string("MPI_AllgatherMPI_AllreduceMPI_AlltoallMPI_Scan")
      + std::string("MPI_Reduce_scatterMPI_Op_createMPI_Op_freeMPIMPI_Alltoallv")
      + std::string("MPI_AllgathervMPI_GathervMPI_Scatterv");

OTFConverter::OTFConverter()
    : rawtrace(NULL), 
      trace(NULL), 
      max_depth(0), 
      globalID(0),
      last_init(0),
      last_finalize(0),
      initFunction(-1),
      finalizeFunction(-1)
{
}

OTFConverter::~OTFConverter()
{
}


Trace * OTFConverter::importOTF(std::string filename)
{
    #ifdef OTF1LIB

    // Start with the rawtrace similar to what we got from PARAVER
    OTFImporter * importer = new OTFImporter();
    rawtrace = importer->importOTF(filename.c_str());

    convert();

    delete importer;
    trace->fullpath = filename;
    #endif
    return trace;
}


Trace * OTFConverter::importOTF2(std::string filename)
{
    // Start with the rawtrace similar to what we got from PARAVER
    OTF2Importer * importer = new OTF2Importer();
    rawtrace = importer->importOTF2(filename.c_str());

    convert();

    delete importer;
    trace->fullpath = filename;
    return trace;
}

void OTFConverter::convert()
{
    // Time the rest of this
    clock_t start = clock();
    trace = new Trace(rawtrace->num_entities, rawtrace->num_pes);
    trace->units = rawtrace->second_magnitude;

    // Start setting up new Trace
    delete trace->functions;
    trace->functions = rawtrace->functions;
    delete trace->functionGroups;
    trace->primaries = rawtrace->primaries;
    trace->processingElements = rawtrace->processingElements;
    trace->functionGroups = rawtrace->functionGroups;
    trace->collectives = rawtrace->collectives;
    trace->collectiveMap = rawtrace->collectiveMap;

    trace->entitygroups = rawtrace->entitygroups;
    trace->collective_definitions = rawtrace->collective_definitions;

    // Find the MPI Group key
    for (std::map<int, std::string>::iterator fxnGroup = trace->functionGroups->begin();
         fxnGroup != trace->functionGroups->end(); ++fxnGroup)
    {
        if (fxnGroup->second.find("MPI") != std::string::npos) {
            trace->mpi_group = fxnGroup->first;
            break;
        }
    }

    // Find init and finalize
    for (std::map<int, Function *>::iterator fxn = trace->functions->begin();
            fxn != trace->functions->end(); ++fxn)
    {
        if (fxn->second->name.compare("MPI_Init") == 0)
        {
            initFunction = fxn->first;
        } 
        else if (fxn->second->name.compare("MPI_Finalize") == 0)
        {
            finalizeFunction = fxn->first;
        }
    }

    // Set up collective metrics
    for (std::map<unsigned int, Counter *>::iterator counter = rawtrace->counters->begin();
         counter != rawtrace->counters->end(); ++counter)
    {
        trace->metrics->push_back((counter->second)->name);
        trace->metric_units->insert(std::pair<std::string, std::string>((counter->second)->name,
                                    (counter->second)->name + " / time"));
    }

    // Convert the events into matching enter and exit
    matchEvents();

    // Sort all the collective records
    for (std::map<unsigned long long, CollectiveRecord *>::iterator cr
         = trace->collectives->begin();
         cr != trace->collectives->end(); ++cr)
    {
        std::sort(cr->second->events->begin(), cr->second->events->end(), Event::eventEntityLessThan);
    }

    trace->last_init = (last_init != 0) ? last_init : trace->min_time;
    trace->last_finalize = (last_finalize != 0) ? last_finalize : trace->max_time;

    trace->max_depth = max_depth;
    clock_t end = clock();
    double traceElapsed = (end - start) / CLOCKS_PER_SEC;
    RavelUtils::gu_printTime(traceElapsed, "Event/Message Matching: ");

    delete rawtrace;
}

// Determine events as blocks of matching enter and exit,
// link them into a call tree
void OTFConverter::matchEvents()
{
    trace->metrics->push_back("Function Count");
    // We can handle each set of events separately
    std::stack<EventRecord *> * stack = new std::stack<EventRecord *>();

    // Keep track of the counters at that time
    std::stack<CounterRecord *> * counterstack = new std::stack<CounterRecord *>();
    std::map<unsigned int, CounterRecord *> * lastcounters = new std::map<unsigned int, CounterRecord *>();

    bool sflag, rflag, isendflag;

    for (int i = 0; i < rawtrace->events->size(); i++)
    {
        std::vector<EventRecord *> * event_list = rawtrace->events->at(i);
        int depth = 0;
        int phase = 0;
        unsigned long long endtime = 0;

        std::vector<CounterRecord *> * counters = rawtrace->counter_records->at(i);
        lastcounters->clear();
        int counter_index = 0;

        std::vector<RawTrace::CollectiveBit *> * collective_bits = rawtrace->collectiveBits->at(i);
        int collective_index = 0;

        std::vector<CommRecord *> * sendlist = rawtrace->messages->at(i);
        std::vector<CommRecord *> * recvlist = rawtrace->messages_r->at(i);
        int sindex = 0, rindex = 0;
        CommEvent * prev = NULL;
        for (std::vector<EventRecord *>::iterator evt = event_list->begin();
             evt != event_list->end(); ++evt)
        {
            if (!((*evt)->enter)) // End of a subroutine
            {
                EventRecord * bgn = stack->top();
                stack->pop();
                if (bgn->time < trace->min_time)
                    trace->min_time = bgn->time;
                if((*evt)->time > trace->max_time)
                    trace->max_time = (*evt)->time;

                // Find init and finalize
                if (bgn->value == initFunction && (*evt)->time > last_init)
                {
                    last_init = (*evt)->time;
                }
                else if (bgn->value == finalizeFunction && bgn->time > last_finalize)
                {
                    last_finalize = bgn->time;
                }

                // Partition/handle comm events
                CollectiveRecord * cr = NULL;
                sflag = false, rflag = false, isendflag = false;
                if (((*(trace->functions))[bgn->value])->group
                        == trace->mpi_group)
                {
                    // Check for possible collective
                    if (collective_index < collective_bits->size()
                        && bgn->time <= collective_bits->at(collective_index)->time
                            && (*evt)->time >= collective_bits->at(collective_index)->time)
                    {
                        cr = collective_bits->at(collective_index)->cr;
                        collective_index++;
                    }

                    // Check/advance sends, including if isend
                    if (sindex < sendlist->size())
                    {
                        if (bgn->time <= sendlist->at(sindex)->send_time
                                && (*evt)->time >= sendlist->at(sindex)->send_time)
                        {
                            sflag = true;
                        }
                        else if (bgn->time > sendlist->at(sindex)->send_time)
                        {
                            std::cout << "Error, skipping message (by send) at ";
                            std::cout << sendlist->at(sindex)->send_time << " on ";
                            std::cout << (*evt)->entity << std::endl;
                            sindex++;
                        }
                    }

                    // Check/advance receives
                    if (rindex < recvlist->size())
                    {
                        if (!sflag && (*evt)->time >= recvlist->at(rindex)->recv_time
                                && bgn->time <= recvlist->at(rindex)->recv_time)
                        {
                            rflag = true;
                        }
                        else if (!sflag && (*evt)->time > recvlist->at(rindex)->recv_time)
                        {
                            std::cout << "Error, skipping message (by recv) at ";
                            std::cout << recvlist->at(rindex)->send_time << " on ";
                            std::cout << (*evt)->entity << std::endl;
                            rindex++;
                        }
                    }
                }

                Event * e = NULL;
                if (cr)
                {
                    cr->events->push_back(new CollectiveEvent(bgn->time, (*evt)->time,
                                            bgn->value, bgn->entity, bgn->entity,
                                            phase, cr));
                    cr->events->back()->setID(globalID++);
                    cr->events->back()->comm_prev = prev;
                    if (prev)
                        prev->comm_next = cr->events->back();
                    prev = cr->events->back();

                    counter_index = advanceCounters(cr->events->back(),
                                                    counterstack,
                                                    counters, counter_index,
                                                    lastcounters);

                    e = cr->events->back();
                }
                else if (sflag)
                {
                    std::vector<Message *> * msgs = new std::vector<Message *>();
                    CommRecord * crec = sendlist->at(sindex);
                    if (!(crec->message))
                    {
                        crec->message = new Message(crec->send_time,
                                                    crec->recv_time,
                                                    crec->group);
                        crec->message->tag = crec->tag;
                        crec->message->size = crec->size;
                    }
                    msgs->push_back(crec->message);
                    crec->message->sender = new P2PEvent(bgn->time, (*evt)->time,
                                                         bgn->value,
                                                         bgn->entity, bgn->entity, phase,
                                                         msgs);
                    crec->message->sender->setID(globalID++);

                    crec->message->sender->comm_prev = prev;
                    if (prev)
                        prev->comm_next = crec->message->sender;
                    prev = crec->message->sender;

                    counter_index = advanceCounters(crec->message->sender,
                                                    counterstack,
                                                    counters, counter_index,
                                                    lastcounters);

                    e = crec->message->sender;
                    sindex++;
                }
                else if (rflag)
                {
                    std::vector<Message *> * msgs = new std::vector<Message *>();
                    CommRecord * crec = NULL;
                    while (rindex < recvlist->size() && (*evt)->time >= recvlist->at(rindex)->recv_time
                           && bgn->time <= recvlist->at(rindex)->recv_time)
                    {
                        crec = recvlist->at(rindex);
                        if (!(crec->message))
                        {
                            crec->message = new Message(crec->send_time,
                                                        crec->recv_time,
                                                        crec->group);
                            crec->message->tag = crec->tag;
                            crec->message->size = crec->size;
                        }
                        msgs->push_back(crec->message);
                        rindex++;
                    }
                    msgs->at(0)->receiver = new P2PEvent(bgn->time, (*evt)->time,
                                                         bgn->value,
                                                         bgn->entity, bgn->entity, phase,
                                                         msgs);
                    msgs->at(0)->receiver->setID(globalID++);
                    for (int i = 1; i < msgs->size(); i++)
                    {
                        msgs->at(i)->receiver = msgs->at(0)->receiver;
                    }
                    msgs->at(0)->receiver->is_recv = true;

                    msgs->at(0)->receiver->comm_prev = prev;
                    if (prev)
                        prev->comm_next = msgs->at(0)->receiver;
                    prev = msgs->at(0)->receiver;

                    counter_index = advanceCounters(msgs->at(0)->receiver,
                                                    counterstack,
                                                    counters, counter_index,
                                                    lastcounters);

                    e = msgs->at(0)->receiver;     
                }
                else // Non-com event
                {
                    e = new Event(bgn->time, (*evt)->time, bgn->value,
                                  bgn->entity, bgn->entity);
                    e->setID(globalID++);


                    // Squelch counter values that we're not keeping track of here (for now)
                    while (!counterstack->empty() && counterstack->top()->time == bgn->time)
                    {
                        counterstack->pop();
                    }
                    while (counters->size() > counter_index
                           && counters->at(counter_index)->time == (*evt)->time)
                    {
                        counter_index++;
                    }
                }

                depth--;
                e->depth = depth;
                if (depth == 0 && !isendflag)
                    (*(trace->roots))[(*evt)->entity]->push_back(e);

                if (e->exit > endtime)
                    endtime = e->exit;
                if (!stack->empty())
                {
                    stack->top()->children.push_back(e);
                }
                for (std::vector<Event *>::iterator child = bgn->children.begin();
                     child != bgn->children.end(); ++child)
                {
                    // If the child already has a caller, it was coalesced.
                    // In that case, we want to make that caller the child
                    // rather than this reality direct one... but only for
                    // the first one
                    if ((*child)->caller)
                    {
                        if (e->callees->empty()
                            || e->callees->back() != (*child)->caller)
                            e->callees->push_back((*child)->caller);
                    }
                    else
                    {
                        e->callees->push_back(*child);
                        (*child)->caller = e;
                    }
                }

                e->addMetric("Function Count", 1);
                (*(trace->events))[(*evt)->entity]->push_back(e);
            }
            else // Begin a subroutine
            {
                depth++;
                if (depth > max_depth)
                {
                    max_depth = depth;
                }
                stack->push(*evt);

                while (counters->size() > counter_index
                       && counters->at(counter_index)->time == (*evt)->time)
                {
                    counterstack->push(counters->at(counter_index));
                    counter_index++;

                    // Set the first one to the beginning of the trace
                    //if (lastcounters->at(counters->at(counter_index)->counter) == NULL)
                    if (lastcounters->find(counters->at(counter_index)->counter) == lastcounters->end())
                    {
                        lastcounters->insert(std::pair<unsigned int, CounterRecord *>(counters->at(counter_index)->counter,
                                             counters->at(counter_index)));
                    }
                }

            }
        }

        // Deal with unclosed trace issues
        // We assume these events are not communication
        while (!stack->empty())
        {
            EventRecord * bgn = stack->top();
            stack->pop();
            endtime = std::max(endtime, bgn->time);
            Event * e = new Event(bgn->time, endtime, bgn->value,
                          bgn->entity, bgn->entity);
            e->setID(globalID++);
            e->addMetric("Function Count", 1);
            if (!stack->empty())
            {
                stack->top()->children.push_back(e);
            }
            for (std::vector<Event *>::iterator child = bgn->children.begin();
                 child != bgn->children.end(); ++child)
            {
                e->callees->push_back(*child);
                (*child)->caller = e;
            }
            (*(trace->events))[bgn->entity]->push_back(e);
            depth--;
        }

        // Prepare for next entity
        while (!stack->empty())
            stack->pop();
    }
    delete stack;
}

// We only do this with comm events right now, so we know we won't have nesting
int OTFConverter::advanceCounters(CommEvent * evt, std::stack<CounterRecord *> * counterstack,
                                   std::vector<CounterRecord *> * counters, int index,
                                   std::map<unsigned int, CounterRecord *> * lastcounters)
{
    CounterRecord * begin, * last, * end;
    int tmpIndex;

    // We know we can't have too many counters recorded, so we can search in them
    while (!counterstack->empty() && counterstack->top()->time == evt->enter)
    {
        begin = counterstack->top();
        counterstack->pop();
        last = lastcounters->at(begin->counter);
        end = NULL;

        tmpIndex = index;
        while(!end && tmpIndex < counters->size())
        {
            if (counters->at(tmpIndex)->time == evt->exit
                    && counters->at(tmpIndex)->counter == begin->counter)
            {
                end = counters->at(tmpIndex);
            }
            tmpIndex++;
        }

        if (end)
        {
            // Add metric
            evt->metrics->addMetric(rawtrace->counters->at(begin->counter)->name,
                                    end->value - begin->value);

            // Update last counters
            lastcounters->insert(std::pair<unsigned int, CounterRecord *>(end->counter, end));

        }
        else
        {
            // Error in some way since matching counter not found
            std::cout << "Matching counter not found!" << std::endl;
        }

    }

    // Advance counters index
    while(index < counters->size()
          && counters->at(index)->time <= evt->exit)
    {
        index++;
    }
    return index;
}
