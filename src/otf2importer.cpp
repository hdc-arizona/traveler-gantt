#include "otf2importer.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include "ravelutils.h"
#include "rawtrace.h"
#include "commrecord.h"
#include "eventrecord.h"
#include "collectiverecord.h"
#include "entitygroup.h"
#include "otfcollective.h"
#include "function.h"
#include "entity.h"
#include "primaryentitygroup.h"

OTF2Importer::OTF2Importer()
    : from_saved_version(""),
      ticks_per_second(0),
      time_offset(0),
      time_conversion_factor(0),
      num_processes(0),
      second_magnitude(1),
      entercount(0),
      exitcount(0),
      sendcount(0),
      recvcount(0),
      otfReader(NULL),
      global_def_callbacks(NULL),
      global_evt_callbacks(NULL),
      stringMap(new std::map<OTF2_StringRef, std::string>()),
      attributeMap(new std::map<OTF2_AttributeRef, OTF2Attribute *>()),
      locationMap(new std::map<OTF2_LocationRef, OTF2Location *>()),
      locationGroupMap(new std::map<OTF2_LocationGroupRef, OTF2LocationGroup *>()),
      regionMap(new std::map<OTF2_RegionRef, OTF2Region *>()),
      commMap(new std::map<OTF2_CommRef, OTF2Comm *>()),
      groupMap(new std::map<OTF2_GroupRef, OTF2Group *>()),
      commIndexMap(new std::map<OTF2_CommRef, int>()),
      regionIndexMap(new std::map<OTF2_RegionRef, int>()),
      locationIndexMap(new std::map<OTF2_LocationRef, unsigned long>()),
      threadList(std::vector<OTF2Location *>()),
      MPILocations(std::set<OTF2_LocationRef>()),
      processingElements(NULL),
      unmatched_recvs(new std::vector<std::list<CommRecord *> *>()),
      unmatched_sends(new std::vector<std::list<CommRecord *> *>()),
      unmatched_send_requests(new std::vector<std::list<CommRecord *> *>()),
      unmatched_send_completes(new std::vector<std::list<OTF2IsendComplete *> *>()),
      rawtrace(NULL),
      primaries(NULL),
      functionGroups(NULL),
      functions(NULL),
      entitygroups(NULL),
      collective_definitions(new std::map<int, OTFCollective *>()),
      counters(NULL),
      collectives(NULL),
      collectiveMap(NULL),
      collective_begins(NULL),
      collective_fragments(NULL),
      metrics(std::vector<OTF2_AttributeRef>()),
      metric_names(new std::vector<std::string>()),
      metric_units(new std::map<std::string, std::string>())
{
    collective_definitions->insert(std::pair<int, OTFCollective*>(0, new OTFCollective(0, 1, "Barrier")));
    collective_definitions->insert(std::pair<int, OTFCollective*>(1, new OTFCollective(1, 2, "Bcast")));
    collective_definitions->insert(std::pair<int, OTFCollective*>(2, new OTFCollective(2, 3, "Gather")));
    collective_definitions->insert(std::pair<int, OTFCollective*>(3, new OTFCollective(3, 3, "Gatherv")));
    collective_definitions->insert(std::pair<int, OTFCollective*>(4, new OTFCollective(4, 2, "Scatter")));
    collective_definitions->insert(std::pair<int, OTFCollective*>(5, new OTFCollective(5, 2, "Scatterv")));
    collective_definitions->insert(std::pair<int, OTFCollective*>(6, new OTFCollective(6, 4, "Allgather")));
    collective_definitions->insert(std::pair<int, OTFCollective*>(7, new OTFCollective(7, 4, "Allgatherv")));
    collective_definitions->insert(std::pair<int, OTFCollective*>(8, new OTFCollective(8, 4, "Alltoall")));
    collective_definitions->insert(std::pair<int, OTFCollective*>(9, new OTFCollective(9, 4, "Alltoallv")));
    collective_definitions->insert(std::pair<int, OTFCollective*>(10, new OTFCollective(10, 4, "Alltoallw")));
    collective_definitions->insert(std::pair<int, OTFCollective*>(11, new OTFCollective(11, 4, "Allreduce")));
    collective_definitions->insert(std::pair<int, OTFCollective*>(12, new OTFCollective(12, 3, "Reduce")));
    collective_definitions->insert(std::pair<int, OTFCollective*>(13, new OTFCollective(13, 4, "ReduceScatter")));
    collective_definitions->insert(std::pair<int, OTFCollective*>(14, new OTFCollective(14, 4, "Scan")));
    collective_definitions->insert(std::pair<int, OTFCollective*>(15, new OTFCollective(15, 4, "Exscan")));
    collective_definitions->insert(std::pair<int, OTFCollective*>(16, new OTFCollective(16, 4, "ReduceScatterBlock")));
    collective_definitions->insert(std::pair<int, OTFCollective*>(17, new OTFCollective(17, 4, "CreateHandle")));
    collective_definitions->insert(std::pair<int, OTFCollective*>(18, new OTFCollective(18, 4, "DestroyHandle")));
    collective_definitions->insert(std::pair<int, OTFCollective*>(19, new OTFCollective(19, 4, "Allocate")));
    collective_definitions->insert(std::pair<int, OTFCollective*>(20, new OTFCollective(20, 4, "Deallocate")));
    collective_definitions->insert(std::pair<int, OTFCollective*>(21, new OTFCollective(21, 4, "CreateAllocate")));
    collective_definitions->insert(std::pair<int, OTFCollective*>(22, new OTFCollective(22, 4, "DestroyDeallocate")));
}

OTF2Importer::~OTF2Importer()
{
    delete stringMap;
    delete collective_begins;

    for (std::vector<std::list<CommRecord *> *>::iterator eitr
         = unmatched_recvs->begin(); eitr != unmatched_recvs->end(); ++eitr)
    {
        for (std::list<CommRecord *>::iterator itr = (*eitr)->begin();
             itr != (*eitr)->end(); ++itr)
        {
            delete *itr;
            *itr = NULL;
        }
        delete *eitr;
        *eitr = NULL;
    }
    delete unmatched_recvs;

    for (std::vector<std::list<CommRecord *> *>::iterator eitr
         = unmatched_sends->begin();
         eitr != unmatched_sends->end(); ++eitr)
    {
        // Don't delete, used elsewhere
        /*for (std::list<CommRecord *>::iterator itr = (*eitr)->begin();
         itr != (*eitr)->end(); ++itr)
        {
            delete *itr;
            *itr = NULL;
        }*/
        delete *eitr;
        *eitr = NULL;
    }
    delete unmatched_sends;


    for (std::vector<std::list<CommRecord *> *>::iterator eitr
         = unmatched_send_requests->begin();
         eitr != unmatched_send_requests->end(); ++eitr)
    {
        // Don't delete, used elsewhere
        /*for (std::list<CommRecord *>::iterator itr = (*eitr)->begin();
         itr != (*eitr)->end(); ++itr)
        {
            delete *itr;
            *itr = NULL;
        }*/
        delete *eitr;
        *eitr = NULL;
    }
    delete unmatched_send_requests;


    for (std::vector<std::list<OTF2IsendComplete *> *>::iterator eitr
         = unmatched_send_completes->begin(); eitr != unmatched_send_completes->end(); ++eitr)
    {
        for (std::list<OTF2IsendComplete *>::iterator itr = (*eitr)->begin();
             itr != (*eitr)->end(); ++itr)
        {
            delete *itr;
            *itr = NULL;
        }
        delete *eitr;
        *eitr = NULL;
    }
    delete unmatched_send_completes;

    for(std::vector<std::list<OTF2CollectiveFragment *> *>::iterator eitr
        = collective_fragments->begin();
        eitr != collective_fragments->end(); ++eitr)
    {
        for (std::list<OTF2CollectiveFragment *>::iterator itr
             = (*eitr)->begin(); itr != (*eitr)->end(); ++itr)
        {
            delete *itr;
            *itr = NULL;
        }
        delete *eitr;
        *eitr = NULL;
    }
    delete collective_fragments;

    for (std::map<OTF2_AttributeRef, OTF2Attribute *>::iterator eitr
         = attributeMap->begin();
         eitr != attributeMap->end(); ++eitr)
    {
        delete eitr->second;
    }
    delete attributeMap;

    for (std::map<OTF2_LocationRef, OTF2Location *>::iterator eitr
         = locationMap->begin();
         eitr != locationMap->end(); ++eitr)
    {
        delete eitr->second;
    }
    delete locationMap;

    for (std::map<OTF2_LocationGroupRef, OTF2LocationGroup *>::iterator eitr
         = locationGroupMap->begin();
         eitr != locationGroupMap->end(); ++eitr)
    {
        delete eitr->second;
    }
    delete locationGroupMap;

    for (std::map<OTF2_RegionRef, OTF2Region *>::iterator eitr
         = regionMap->begin();
         eitr != regionMap->end(); ++eitr)
    {
        delete eitr->second;
    }
    delete regionMap;

    // Don't delete members from OTF2Group, those becomes
    // processes in kept communicators
    for (std::map<OTF2_GroupRef, OTF2Group *>::iterator eitr
         = groupMap->begin();
         eitr != groupMap->end(); ++eitr)
    {
        delete eitr->second;
    }
    delete groupMap;

    for (std::map<OTF2_CommRef, OTF2Comm *>::iterator eitr
         = commMap->begin();
         eitr != commMap->end(); ++eitr)
    {
        delete eitr->second;
    }
    delete commMap;
}

RawTrace * OTF2Importer::importOTF2(const char* otf_file)
{
    entercount = 0;
    exitcount = 0;
    sendcount = 0;
    recvcount = 0;

    clock_t start = clock();

    // Setup
    otfReader = OTF2_Reader_Open(otf_file);
    OTF2_Reader_SetSerialCollectiveCallbacks(otfReader);
    OTF2_GlobalDefReader * global_def_reader = OTF2_Reader_GetGlobalDefReader(otfReader);
    global_def_callbacks = OTF2_GlobalDefReaderCallbacks_New();

    setDefCallbacks();

    OTF2_Reader_RegisterGlobalDefCallbacks( otfReader,
                                            global_def_reader,
                                            global_def_callbacks,
                                            this ); // Register userdata as this

    OTF2_GlobalDefReaderCallbacks_Delete( global_def_callbacks );

    // Read definitions
    std::cout << "Reading definitions" << std::endl;
    uint64_t definitions_read = 0;
    OTF2_Reader_ReadAllGlobalDefinitions( otfReader,
                                          global_def_reader,
                                          &definitions_read );


    primaries = new std::map<int, PrimaryEntityGroup *>();
    functionGroups = new std::map<int, std::string>();
    functions = new std::map<int, Function *>();
    entitygroups = new std::map<int, EntityGroup *>();
    collectives = new std::map<unsigned long long, CollectiveRecord *>();
    counters = new std::map<unsigned int, Counter *>();

    processDefinitions();

    rawtrace = new RawTrace(num_processes, num_processes);
    rawtrace->primaries = primaries;
    rawtrace->second_magnitude = second_magnitude;
    rawtrace->functions = functions;
    rawtrace->functionGroups = functionGroups;
    rawtrace->entitygroups = entitygroups;
    rawtrace->collective_definitions = collective_definitions;
    rawtrace->collectives = collectives;
    rawtrace->counters = counters;
    rawtrace->events = new std::vector<std::vector<EventRecord *> *>(num_processes);
    rawtrace->messages = new std::vector<std::vector<CommRecord *> *>(num_processes);
    rawtrace->messages_r = new std::vector<std::vector<CommRecord *> *>(num_processes);
    rawtrace->counter_records = new std::vector<std::vector<CounterRecord *> *>(num_processes);
    rawtrace->collectiveBits = new std::vector<std::vector<RawTrace::CollectiveBit *> *>(num_processes);
    rawtrace->metric_names = metric_names;
    rawtrace->metric_units = metric_units;

    // Adding locations
    // Use the locationIndexMap to only chooes the ones we can handle (right now just MPI)
    for (std::map<OTF2_LocationRef, unsigned long>::iterator loc = locationIndexMap->begin();
         loc != locationIndexMap->end(); ++loc)
    {
        OTF2_Reader_SelectLocation(otfReader, loc->first);
    }

    bool def_files_success = OTF2_Reader_OpenDefFiles(otfReader) == OTF2_SUCCESS;
    OTF2_Reader_OpenEvtFiles(otfReader);
    for (std::map<OTF2_LocationRef, unsigned long>::iterator loc = locationIndexMap->begin();
         loc != locationIndexMap->end(); ++loc)
    {
        if (def_files_success)
        {
            OTF2_DefReader * def_reader = OTF2_Reader_GetDefReader(otfReader, loc->first);
            if (def_reader)
            {
                uint64_t def_reads = 0;
                OTF2_Reader_ReadAllLocalDefinitions( otfReader,
                                                     def_reader,
                                                     &def_reads );
                OTF2_Reader_CloseDefReader( otfReader, def_reader );
            }
        }
        // Required line, though unused
        OTF2_EvtReader * unused = OTF2_Reader_GetEvtReader(otfReader, loc->first);
    }
    if (def_files_success)
        OTF2_Reader_CloseDefFiles(otfReader);

    std::cout << "Reading events" << std::endl;
    delete unmatched_recvs;
    unmatched_recvs = new std::vector<std::list<CommRecord *> *>(num_processes);
    delete unmatched_sends;
    unmatched_sends = new std::vector<std::list<CommRecord *> *>(num_processes);
    delete unmatched_send_requests;
    unmatched_send_requests = new std::vector<std::list<CommRecord *> *>(num_processes);
    delete unmatched_send_completes;
    unmatched_send_completes = new std::vector<std::list<OTF2IsendComplete *> *>(num_processes);
    delete collectiveMap;
    collectiveMap = new std::vector<std::map<unsigned long long, CollectiveRecord *> *>(num_processes);
    delete collective_begins;
    collective_begins = new std::vector<std::list<uint64_t> *>(num_processes);
    delete collective_fragments;
    collective_fragments = new std::vector<std::list<OTF2CollectiveFragment *> *>(num_processes);
    for (int i = 0; i < num_processes; i++) {
        (*unmatched_recvs)[i] = new std::list<CommRecord *>();
        (*unmatched_sends)[i] = new std::list<CommRecord *>();
        (*unmatched_send_requests)[i] = new std::list<CommRecord *>();
        (*unmatched_send_completes)[i] = new std::list<OTF2IsendComplete *>();
        (*collectiveMap)[i] = new std::map<unsigned long long, CollectiveRecord *>();
        (*(rawtrace->events))[i] = new std::vector<EventRecord *>();
        (*(rawtrace->messages))[i] = new std::vector<CommRecord *>();
        (*(rawtrace->messages_r))[i] = new std::vector<CommRecord *>();
        (*(rawtrace->counter_records))[i] = new std::vector<CounterRecord *>();
        (*collective_begins)[i] = new std::list<uint64_t>();
        (*collective_fragments)[i] = new std::list<OTF2CollectiveFragment *>();
        (*(rawtrace->collectiveBits))[i] = new std::vector<RawTrace::CollectiveBit *>();
    }


    OTF2_GlobalEvtReader * global_evt_reader = OTF2_Reader_GetGlobalEvtReader(otfReader);

    global_evt_callbacks = OTF2_GlobalEvtReaderCallbacks_New();

    setEvtCallbacks();

    OTF2_Reader_RegisterGlobalEvtCallbacks( otfReader,
                                            global_evt_reader,
                                            global_evt_callbacks,
                                            this ); // Register userdata as this

    OTF2_GlobalEvtReaderCallbacks_Delete( global_evt_callbacks );
    uint64_t events_read = 0;
    OTF2_Reader_ReadAllGlobalEvents( otfReader,
                                     global_evt_reader,
                                     &events_read );


    processCollectives();

    rawtrace->collectiveMap = collectiveMap;

    OTF2_Reader_CloseGlobalEvtReader( otfReader, global_evt_reader );
    OTF2_Reader_CloseEvtFiles( otfReader );
    OTF2_Reader_Close( otfReader );

    std::cout << "Finish reading" << std::endl;

    int unmatched_recv_count = 0;
    for (std::vector<std::list<CommRecord *> *>::iterator eitr
         = unmatched_recvs->begin();
         eitr != unmatched_recvs->end(); ++eitr)
    {
        for (std::list<CommRecord *>::iterator itr = (*eitr)->begin();
             itr != (*eitr)->end(); ++itr)
        {
            unmatched_recv_count++;
            std::cout << "Unmatched RECV " << (*itr)->sender << "->"
                      << (*itr)->receiver << " (" << (*itr)->send_time << ", "
                      << (*itr)->recv_time << ")" << std::endl;
        }
    }
    int unmatched_send_count = 0;
    for (std::vector<std::list<CommRecord *> *>::iterator eitr
         = unmatched_sends->begin();
         eitr != unmatched_sends->end(); ++eitr)
    {
        for (std::list<CommRecord *>::iterator itr = (*eitr)->begin();
             itr != (*eitr)->end(); ++itr)
        {
            unmatched_send_count++;
            std::cout << "Unmatched SEND " << (*itr)->sender << "->"
                      << (*itr)->receiver << " (" << (*itr)->send_time << ", "
                      << (*itr)->recv_time << ")" << std::endl;
        }
    }
    std::cout << unmatched_send_count << " unmatched sends and "
              << unmatched_recv_count << " unmatched recvs." << std::endl;


    defineEntities();
    rawtrace->processingElements = processingElements;
    rawtrace->num_entities = MPILocations.size();

    clock_t end = clock();
    double traceElapsed = (end - start) / CLOCKS_PER_SEC;
    RavelUtils::gu_printTime(traceElapsed, "OTF Reading: ");

    return rawtrace;
}

void OTF2Importer::setDefCallbacks()
{
    // String
    OTF2_GlobalDefReaderCallbacks_SetStringCallback(global_def_callbacks,
                                                    callbackDefString);

    // Attributes
    OTF2_GlobalDefReaderCallbacks_SetAttributeCallback(global_def_callbacks,
                                                       callbackDefAttribute);

    // Timer
    OTF2_GlobalDefReaderCallbacks_SetClockPropertiesCallback(global_def_callbacks,
                                                             callbackDefClockProperties);

    // Locations
    OTF2_GlobalDefReaderCallbacks_SetLocationGroupCallback(global_def_callbacks,
                                                           callbackDefLocationGroup);
    OTF2_GlobalDefReaderCallbacks_SetLocationCallback(global_def_callbacks,
                                                      callbackDefLocation);

    // Comm
    OTF2_GlobalDefReaderCallbacks_SetCommCallback(global_def_callbacks,
                                                  callbackDefComm);
    OTF2_GlobalDefReaderCallbacks_SetGroupCallback(global_def_callbacks,
                                                  callbackDefGroup);

    // Region
    OTF2_GlobalDefReaderCallbacks_SetRegionCallback(global_def_callbacks,
                                                    callbackDefRegion);


    // TODO: Metrics might be akin to counters
}

void OTF2Importer::defineEntities()
{
    // Grab only the MPI locations
    primaries->insert(std::pair<int, PrimaryEntityGroup *>(0, new PrimaryEntityGroup(0, "MPI")));
    std::map<OTF2_LocationRef, Entity *> entityMap = std::map<OTF2_LocationRef, Entity *>();
    for (std::map<OTF2_LocationRef, OTF2Location *>::iterator loc = locationMap->begin();
         loc != locationMap->end(); ++loc)
    {
        OTF2_LocationGroupType group = (locationGroupMap->at((loc->second)->group))->type;
        if (group == OTF2_LOCATION_GROUP_TYPE_PROCESS)
        {
            OTF2_LocationType type = (loc->second)->type;
            if (type == OTF2_LOCATION_TYPE_CPU_THREAD)
            {
                if (MPILocations.find(loc->first) != MPILocations.end())
                {
                    unsigned long entity = locationIndexMap->at(loc->first);
                    Entity * locationEntity = new Entity(entity,
                                                         std::to_string(loc->second->group),
                                                         primaries->at(0));
                    primaries->at(0)->entities->insert(primaries->at(0)->entities->begin() + entity, locationEntity);
                    entityMap.insert(std::pair<OTF2_LocationRef, Entity *>(loc->first, locationEntity));
                }
            }
        }
    }

    processingElements = new PrimaryEntityGroup(1, "PEs");
    std::sort(threadList.begin(), threadList.end());
    for (std::vector<OTF2Location *>::iterator loc = threadList.begin();
         loc != threadList.end(); ++loc)
    {
        if (MPILocations.find((*loc)->self) != MPILocations.end())
        {
            processingElements->entities->push_back(entityMap.at((*loc)->self));
        }
        else
        {
            unsigned long entity = locationIndexMap->at((*loc)->self);
            Entity * locationEntity = new Entity(entity,
                                                 stringMap->at((*loc)->name),
                                                 processingElements);
            processingElements->entities->push_back(locationEntity);
        }
    }

}

void OTF2Importer::processDefinitions()
{
    int index = 0;
    for (std::map<OTF2_RegionRef, OTF2Region *>::iterator region = regionMap->begin();
         region != regionMap->end(); ++region)
    {
        regionIndexMap->insert(std::pair<OTF2_RegionRef, int>(region->first, index));
        functions->insert(std::pair<int, Function *>(index, new Function(stringMap->at(region->second->name),
                                              (region->second)->paradigm)));
        index++;
    }

    functionGroups->insert(std::pair<int, std::string>(OTF2_PARADIGM_MPI, "MPI"));

    // Grab only the PE locations
    for (std::map<OTF2_LocationRef, OTF2Location *>::iterator loc = locationMap->begin();
         loc != locationMap->end(); ++loc)
    {
        OTF2_LocationGroupType group = (locationGroupMap->at((loc->second)->group))->type;
        if (group == OTF2_LOCATION_GROUP_TYPE_PROCESS)
        {
            OTF2_LocationType type = (loc->second)->type;
            if (type == OTF2_LOCATION_TYPE_CPU_THREAD)
            {
                threadList.push_back(loc->second);
                locationIndexMap->insert(std::pair<OTF2_LocationRef, unsigned long>(loc->first, threadList.size() - 1));
            }
        }
    }
    num_processes = threadList.size();

    index = 0;
    for (std::map<OTF2_CommRef, OTF2Comm *>::iterator comm = commMap->begin();
         comm != commMap->end(); ++comm)
    {
        commIndexMap->insert(std::pair<OTF2_CommRef, int>(comm->first, index));
        EntityGroup * t = new EntityGroup(index, stringMap->at((comm->second)->name));
        //delete t->entities;
        //t->entities = groupMap->value((comm.value())->group)->members;
        for (int i = 0; i < groupMap->at((comm->second)->group)->members->size(); i++)
        {
            t->entityorder->insert(std::pair<unsigned long, int>(groupMap->at((comm->second)->group)->members->at(i), i));
            t->entities->push_back(groupMap->at((comm->second)->group)->members->at(i));
        }
        entitygroups->insert(std::pair<int, EntityGroup *>(index, t));
        index++;
    }
}

void OTF2Importer::setEvtCallbacks()
{
    // Enter / Leave
    OTF2_GlobalEvtReaderCallbacks_SetEnterCallback(global_evt_callbacks,
                                                   &OTF2Importer::callbackEnter);
    OTF2_GlobalEvtReaderCallbacks_SetLeaveCallback(global_evt_callbacks,
                                                   &OTF2Importer::callbackLeave);


    // P2P
    OTF2_GlobalEvtReaderCallbacks_SetMpiSendCallback(global_evt_callbacks,
                                                     &OTF2Importer::callbackMPISend);
    OTF2_GlobalEvtReaderCallbacks_SetMpiIsendCallback(global_evt_callbacks,
                                                      &OTF2Importer::callbackMPIIsend);
    OTF2_GlobalEvtReaderCallbacks_SetMpiIsendCompleteCallback(global_evt_callbacks,
                                                              &OTF2Importer::callbackMPIIsendComplete);
    //OTF2_GlobalEvtReaderCallbacks_SetMpiIrecvRequestCallback(global_evt_callbacks,
    //                                                         &OTF2Importer::callbackMPIIrecvRequest);
    OTF2_GlobalEvtReaderCallbacks_SetMpiIrecvCallback(global_evt_callbacks,
                                                      &OTF2Importer::callbackMPIIrecv);
    OTF2_GlobalEvtReaderCallbacks_SetMpiRecvCallback(global_evt_callbacks,
                                                     &OTF2Importer::callbackMPIRecv);


    // Collective
    OTF2_GlobalEvtReaderCallbacks_SetMpiCollectiveBeginCallback(global_evt_callbacks,
                                                                  callbackMPICollectiveBegin);
    OTF2_GlobalEvtReaderCallbacks_SetMpiCollectiveEndCallback(global_evt_callbacks,
                                                               callbackMPICollectiveEnd);



}

// Find timescale
uint64_t OTF2Importer::convertTime(void* userData, OTF2_TimeStamp time)
{
    return (uint64_t) ((double) (time - ((OTF2Importer *) userData)->time_offset))
            * ((OTF2Importer *) userData)->time_conversion_factor;
}


// May want to save globalOffset and traceLength for max and min
OTF2_CallbackCode OTF2Importer::callbackDefClockProperties(void * userData,
                                                           uint64_t timerResolution,
                                                           uint64_t globalOffset,
                                                           uint64_t traceLength)
{
    ((OTF2Importer*) userData)->ticks_per_second = timerResolution;
    ((OTF2Importer*) userData)->time_offset = globalOffset;
    ((OTF2Importer*) userData)->second_magnitude
            = (int) floor(log10(timerResolution));

    // Use the timer resolution to convert to seconds and
    // multiply by the magnitude of this factor to get into
    // fractions of a second befitting the recorded unit.
    double conversion_factor;
    conversion_factor = pow(10,
                            ((OTF2Importer*) userData)->second_magnitude) // Convert to ms, ns, fs etc
            / ((double) timerResolution); // Convert to seconds

    ((OTF2Importer*) userData)->time_conversion_factor = conversion_factor;
    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Importer::callbackDefString(void * userData,
                                                  OTF2_StringRef self,
                                                  const char * string)
{
    ((OTF2Importer*) userData)->stringMap->insert(std::pair<OTF2_StringRef, std::string>(self, std::string(string)));
    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Importer::callbackDefAttribute(void * userData,
                                                     OTF2_AttributeRef self,
                                                     OTF2_StringRef name,
                                                     OTF2_StringRef description,
                                                     OTF2_Type type)
{
    OTF2Attribute * a = new OTF2Attribute(self, name, description, type);
    ((OTF2Importer *) userData)->attributeMap->insert(std::pair<OTF2_AttributeRef, OTF2Attribute *>(self, a));
    return OTF2_CALLBACK_SUCCESS;
}


OTF2_CallbackCode OTF2Importer::callbackDefLocationGroup(void * userData,
                                                         OTF2_LocationGroupRef self,
                                                         OTF2_StringRef name,
                                                         OTF2_LocationGroupType locationGroupType,
                                                         OTF2_SystemTreeNodeRef systemTreeParent)
{
    OTF2LocationGroup * g = new OTF2LocationGroup(self, name, locationGroupType,
                                                  systemTreeParent);
    (*(((OTF2Importer*) userData)->locationGroupMap))[self] = g;
    return OTF2_CALLBACK_SUCCESS;
}


OTF2_CallbackCode OTF2Importer::callbackDefLocation(void * userData,
                                                    OTF2_LocationRef self,
                                                    OTF2_StringRef name,
                                                    OTF2_LocationType locationType,
                                                    uint64_t numberOfEvents,
                                                    OTF2_LocationGroupRef locationGroup)
{
    OTF2Location * loc = new OTF2Location(self, name, locationType,
                                          numberOfEvents, locationGroup);
    (*(((OTF2Importer*) userData)->locationMap))[self] = loc;

    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Importer::callbackDefComm(void * userData,
                                                OTF2_CommRef self,
                                                OTF2_StringRef name,
                                                OTF2_GroupRef group,
                                                OTF2_CommRef parent)
{

    OTF2Comm * c = new OTF2Comm(self, name, group, parent);
    (*(((OTF2Importer*) userData)->commMap))[self] = c;

    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Importer::callbackDefRegion(void * userData,
                                                  OTF2_RegionRef self,
                                                  OTF2_StringRef name,
                                                  OTF2_StringRef canonicalName,
                                                  OTF2_StringRef description,
                                                  OTF2_RegionRole regionRole,
                                                  OTF2_Paradigm paradigm,
                                                  OTF2_RegionFlag regionFlag,
                                                  OTF2_StringRef sourceFile,
                                                  uint32_t beginLineNumber,
                                                  uint32_t endLineNumber)
{
    OTF2Region * r = new OTF2Region(self, name, canonicalName, regionRole,
                                    paradigm, regionFlag, sourceFile,
                                    beginLineNumber, endLineNumber);
     (*(((OTF2Importer*) userData)->regionMap))[self] = r;
    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Importer::callbackDefGroup(void* userData,
                                                 OTF2_GroupRef self,
                                                 OTF2_StringRef name,
                                                 OTF2_GroupType groupType,
                                                 OTF2_Paradigm paradigm,
                                                 OTF2_GroupFlag groupFlags,
                                                 uint32_t numberOfMembers,
                                                 const uint64_t* members)
{
    OTF2Group * g = new OTF2Group(self, name, groupType, paradigm, groupFlags);
    for (uint32_t i = 0; i < numberOfMembers; i++)
        g->members->push_back(members[i]);
    (*(((OTF2Importer*) userData)->groupMap))[self] = g;
    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Importer::callbackEnter(OTF2_LocationRef locationID,
                                              OTF2_TimeStamp time,
                                              void * userData,
                                              OTF2_AttributeList * attributeList,
                                              OTF2_RegionRef region)
{
    unsigned long location = ((OTF2Importer *) userData)->locationIndexMap->at(locationID);
    int function = ((OTF2Importer *) userData)->regionIndexMap->at(region);
    ((*((((OTF2Importer*) userData)->rawtrace)->events))[location])->push_back(new EventRecord(location,
                                                                                           convertTime(userData,
                                                                                                       time),
                                                                                           function,
                                                                                           true));
    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Importer::callbackLeave(OTF2_LocationRef locationID,
                                              OTF2_TimeStamp time,
                                              void * userData,
                                              OTF2_AttributeList * attributeList,
                                              OTF2_RegionRef region)
{
    unsigned long location = ((OTF2Importer *) userData)->locationIndexMap->at(locationID);
    int function = ((OTF2Importer *) userData)->regionIndexMap->at(region);
    EventRecord * er = new EventRecord(location,
                                       convertTime(userData, time),
                                       function,
                                       false);
    ((*((((OTF2Importer*) userData)->rawtrace)->events))[location])->push_back(er);

    return OTF2_CALLBACK_SUCCESS;
}


// Check if two comm records match
// (one that already is a record, one that is just parts)
bool OTF2Importer::compareComms(CommRecord * comm, unsigned long sender,
                                unsigned long receiver, unsigned int tag,
                                unsigned int size)
{
    if ((comm->sender != sender) || (comm->receiver != receiver)
            || (comm->tag != tag) || (comm->size != size))
        return false;
    return true;
}

bool OTF2Importer::compareComms(CommRecord * comm, unsigned long sender,
                                unsigned long receiver, unsigned int tag)
{
    if ((comm->sender != sender) || (comm->receiver != receiver)
            || (comm->tag != tag))
        return false;
    return true;
}


OTF2_CallbackCode OTF2Importer::callbackMPISend(OTF2_LocationRef locationID,
                                                OTF2_TimeStamp time,
                                                void * userData,
                                                OTF2_AttributeList * attributeList,
                                                uint32_t receiver,
                                                OTF2_CommRef communicator,
                                                uint32_t msgTag,
                                                uint64_t msgLength)
{
    ((OTF2Importer *) userData)->MPILocations.insert(locationID);

    // Every time we find a send, check the unmatched recvs
    // to see if it has a match
    unsigned long long converted_time = convertTime(userData, time);
    unsigned long sender = ((OTF2Importer *) userData)->locationIndexMap->at(locationID);
    OTF2Comm * comm = ((OTF2Importer *) userData)->commMap->at(communicator);
    OTF2Group * group = ((OTF2Importer *) userData)->groupMap->at(comm->group);
    unsigned long world_receiver = group->members->at(receiver);
    CommRecord * cr = NULL;
    std::list<CommRecord *> * unmatched = (*(((OTF2Importer *) userData)->unmatched_recvs))[sender];
    for (std::list<CommRecord *>::iterator itr = unmatched->begin();
         itr != unmatched->end(); ++itr)
    {
        if (OTF2Importer::compareComms((*itr), sender, world_receiver, msgTag))
        {
            cr = *itr;
            cr->send_time = converted_time;
            ((*((((OTF2Importer*) userData)->rawtrace)->messages))[sender])->push_back((cr));
            break;
        }
    }


    // If we did find a match, remove it from the unmatched.
    // Otherwise, create a new unmatched send record
    if (cr)
    {
        std::list<CommRecord *>::iterator delit
                = std::find((*(((OTF2Importer *) userData)->unmatched_recvs))[sender]->begin(),
                            (*(((OTF2Importer *) userData)->unmatched_recvs))[sender]->end(),
                            cr);
        if (delit != (*(((OTF2Importer *) userData)->unmatched_recvs))[sender]->end())
            (*(((OTF2Importer *) userData)->unmatched_recvs))[sender]->erase(delit);

        //(*(((OTF2Importer *) userData)->unmatched_recvs))[sender]->removeOne(cr);
    }
    else
    {
        int entitygroup = ((OTF2Importer *) userData)->commIndexMap->at(communicator);
        cr = new CommRecord(sender, converted_time, world_receiver, 0, msgLength, msgTag, entitygroup);
        (*((((OTF2Importer*) userData)->rawtrace)->messages))[sender]->push_back(cr);
        (*(((OTF2Importer *) userData)->unmatched_sends))[sender]->push_back(cr);
    }
    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Importer::callbackMPIIsend(OTF2_LocationRef locationID,
                                                 OTF2_TimeStamp time,
                                                 void * userData,
                                                 OTF2_AttributeList * attributeList,
                                                 uint32_t receiver,
                                                 OTF2_CommRef communicator,
                                                 uint32_t msgTag,
                                                 uint64_t msgLength,
                                                 uint64_t requestID)
{
    ((OTF2Importer *) userData)->MPILocations.insert(locationID);

    // Every time we find a send, check the unmatched recvs
    // to see if it has a match
    unsigned long long converted_time = convertTime(userData, time);
    unsigned long sender = ((OTF2Importer *) userData)->locationIndexMap->at(locationID);
    CommRecord * cr = NULL;
    std::list<CommRecord *> * unmatched = (*(((OTF2Importer *) userData)->unmatched_recvs))[sender];
    for (std::list<CommRecord *>::iterator itr = unmatched->begin();
         itr != unmatched->end(); ++itr)
    {
        if (OTF2Importer::compareComms((*itr), sender, receiver, msgTag))
        {
            cr = *itr;
            cr->send_time = converted_time;
            ((*((((OTF2Importer*) userData)->rawtrace)->messages))[sender])->push_back((cr));
            break;
        }
    }

    // If we did find a match, remove it from the unmatched.
    // Otherwise, create a new unmatched send record
    if (cr)
    {
        std::list<CommRecord *>::iterator delit
                = std::find((*(((OTF2Importer *) userData)->unmatched_recvs))[sender]->begin(),
                            (*(((OTF2Importer *) userData)->unmatched_recvs))[sender]->end(),
                            cr);
        if (delit != (*(((OTF2Importer *) userData)->unmatched_recvs))[sender]->end())
            (*(((OTF2Importer *) userData)->unmatched_recvs))[sender]->erase(delit);

        //(*(((OTF2Importer *) userData)->unmatched_recvs))[sender]->removeOne(cr);
    }
    else
    {
        int entitygroup = ((OTF2Importer *) userData)->commIndexMap->at(communicator);
        cr = new CommRecord(sender, converted_time, receiver, 0, msgLength,
                            msgTag, entitygroup, requestID);
        (*((((OTF2Importer*) userData)->rawtrace)->messages))[sender]->push_back(cr);
        (*(((OTF2Importer *) userData)->unmatched_sends))[sender]->push_back(cr);
    }

    // Also check the complete time stuff
    OTF2IsendComplete * complete = NULL;
    std::list<OTF2IsendComplete *> * completes
            = (*(((OTF2Importer *) userData)->unmatched_send_completes))[sender];
    for (std::list<OTF2IsendComplete *>::iterator itr = completes->begin();
         itr != completes->end(); ++itr)
    {
        if (requestID ==(*itr)->request)
        {
            complete = *itr;
            cr->send_complete = (*itr)->time;
            break;
        }
    }

    if (complete)
    {
        std::list<OTF2IsendComplete *>::iterator delit
                = std::find((*(((OTF2Importer *) userData)->unmatched_send_completes))[sender]->begin(),
                            (*(((OTF2Importer *) userData)->unmatched_send_completes))[sender]->end(),
                            complete);
        if (delit != (*(((OTF2Importer *) userData)->unmatched_send_completes))[sender]->end())
            (*(((OTF2Importer *) userData)->unmatched_send_completes))[sender]->erase(delit);
        //(*(((OTF2Importer *) userData)->unmatched_send_completes))[sender]->removeOne(complete);
    }
    else
    {
        (*(((OTF2Importer *) userData)->unmatched_send_requests))[sender]->push_back(cr);
    }

    return OTF2_CALLBACK_SUCCESS;
}


// Do nothing for now
OTF2_CallbackCode OTF2Importer::callbackMPIIsendComplete(OTF2_LocationRef locationID,
                                                         OTF2_TimeStamp time,
                                                         void * userData,
                                                         OTF2_AttributeList * attributeList,
                                                         uint64_t requestID)
{
    // Check to see if we have a matching send request
    unsigned long long converted_time = convertTime(userData, time);
    unsigned long sender = ((OTF2Importer *) userData)->locationIndexMap->at(locationID);
    CommRecord * cr = NULL;
    std::list<CommRecord *> * unmatched = (*(((OTF2Importer *) userData)->unmatched_send_requests))[sender];
    for (std::list<CommRecord *>::iterator itr = unmatched->begin();
         itr != unmatched->end(); ++itr)
    {
        if ((*itr)->send_request == requestID)
        {
            cr = *itr;
            cr->send_complete = converted_time;
            break;
        }
    }

    // If we did find a match, remove it from the unmatched.
    // Otherwise, create a new unmatched send record
    if (cr)
    {
        std::list<CommRecord *>::iterator delit
                = std::find((*(((OTF2Importer *) userData)->unmatched_send_requests))[sender]->begin(),
                            (*(((OTF2Importer *) userData)->unmatched_send_requests))[sender]->end(),
                            cr);
        if (delit != (*(((OTF2Importer *) userData)->unmatched_send_requests))[sender]->end())
            (*(((OTF2Importer *) userData)->unmatched_send_requests))[sender]->erase(delit);
        //(*(((OTF2Importer *) userData)->unmatched_send_requests))[sender]->removeOne(cr);
    }
    else
    {
        (*(((OTF2Importer *) userData)->unmatched_send_completes))[sender]->push_back(new OTF2IsendComplete(converted_time,
                                                                                                         requestID));
    }

    return OTF2_CALLBACK_SUCCESS;
}

// Do nothing for now
OTF2_CallbackCode OTF2Importer::callbackMPIIrecvRequest(OTF2_LocationRef locationID,
                                                        OTF2_TimeStamp time,
                                                        void * userData,
                                                        OTF2_AttributeList * attributeList,
                                                        uint64_t requestID)
{
    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Importer::callbackMPIRecv(OTF2_LocationRef locationID,
                                                OTF2_TimeStamp time,
                                                void * userData,
                                                OTF2_AttributeList * attributeList,
                                                uint32_t sender,
                                                OTF2_CommRef communicator,
                                                uint32_t msgTag,
                                                uint64_t msgLength)
{
    ((OTF2Importer *) userData)->MPILocations.insert(locationID);

    // Look for match in unmatched_sends
    unsigned long long converted_time = convertTime(userData, time);
    unsigned long receiver = ((OTF2Importer *) userData)->locationIndexMap->at(locationID);
    OTF2Comm * comm = ((OTF2Importer *) userData)->commMap->at(communicator);
    OTF2Group * group = ((OTF2Importer *) userData)->groupMap->at(comm->group);
    unsigned long world_sender = group->members->at(sender);
    CommRecord * cr = NULL;
    std::list<CommRecord *> * unmatched = (*(((OTF2Importer*) userData)->unmatched_sends))[world_sender];
    for (std::list<CommRecord *>::iterator itr = unmatched->begin();
         itr != unmatched->end(); ++itr)
    {
        if (OTF2Importer::compareComms((*itr), world_sender, receiver, msgTag))
        {
            cr = *itr;
            cr->recv_time = converted_time;
            break;
        }
    }

    // If match is found, remove it from unmatched_sends, otherwise create
    // a new unmatched recv record
    if (cr)
    {
        std::list<CommRecord *>::iterator delit
                = std::find((*(((OTF2Importer *) userData)->unmatched_sends))[world_sender]->begin(),
                            (*(((OTF2Importer *) userData)->unmatched_sends))[world_sender]->end(),
                            cr);
        if (delit != (*(((OTF2Importer *) userData)->unmatched_sends))[world_sender]->end())
            (*(((OTF2Importer *) userData)->unmatched_sends))[world_sender]->erase(delit);
        //(*(((OTF2Importer *) userData)->unmatched_sends))[world_sender]->removeOne(cr);
    }
    else
    {
        int entitygroup = ((OTF2Importer *) userData)->commIndexMap->at(communicator);
        cr = new CommRecord(world_sender, 0, receiver, converted_time, msgLength, msgTag, entitygroup);
        ((*(((OTF2Importer*) userData)->unmatched_recvs))[world_sender])->push_back(cr);
    }
    (*((((OTF2Importer*) userData)->rawtrace)->messages_r))[receiver]->push_back(cr);

    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Importer::callbackMPIIrecv(OTF2_LocationRef locationID,
                                                 OTF2_TimeStamp time,
                                                 void * userData,
                                                 OTF2_AttributeList * attributeList,
                                                 uint32_t sender,
                                                 OTF2_CommRef communicator,
                                                 uint32_t msgTag,
                                                 uint64_t msgLength,
                                                 uint64_t requestID)
{
    ((OTF2Importer *) userData)->MPILocations.insert(locationID);

    // Look for match in unmatched_sends
    unsigned long long converted_time = convertTime(userData, time);
    unsigned long receiver = ((OTF2Importer *) userData)->locationIndexMap->at(locationID);
    CommRecord * cr = NULL;
    std::list<CommRecord *> * unmatched = (*(((OTF2Importer*) userData)->unmatched_sends))[sender];
    for (std::list<CommRecord *>::iterator itr = unmatched->begin();
         itr != unmatched->end(); ++itr)
    {
        if (OTF2Importer::compareComms((*itr), sender, receiver, msgTag))
        {
            cr = *itr;
            cr->recv_time = converted_time;
            break;
        }
    }

    // If match is found, remove it from unmatched_sends, otherwise create
    // a new unmatched recv record
    if (cr)
    {
        std::list<CommRecord *>::iterator delit
                = std::find((*(((OTF2Importer *) userData)->unmatched_sends))[sender]->begin(),
                            (*(((OTF2Importer *) userData)->unmatched_sends))[sender]->end(),
                            cr);
        if (delit != (*(((OTF2Importer *) userData)->unmatched_sends))[sender]->end())
            (*(((OTF2Importer *) userData)->unmatched_sends))[sender]->erase(delit);
        //(*(((OTF2Importer *) userData)->unmatched_sends))[sender]->removeOne(cr);
    }
    else
    {
        int entitygroup = ((OTF2Importer *) userData)->commIndexMap->at(communicator);
        cr = new CommRecord(sender, 0, receiver, converted_time, msgLength, msgTag, entitygroup);
        ((*(((OTF2Importer*) userData)->unmatched_recvs))[sender])->push_back(cr);
    }
    (*((((OTF2Importer*) userData)->rawtrace)->messages_r))[receiver]->push_back(cr);

    return OTF2_CALLBACK_SUCCESS;
}

// We have to just collect the Collective information for now and then go through
// it in order later because we are not guaranteed on order for begin/end and
// interleaving between processes.
OTF2_CallbackCode OTF2Importer::callbackMPICollectiveBegin(OTF2_LocationRef locationID,
                                                           OTF2_TimeStamp time,
                                                           void * userData,
                                                           OTF2_AttributeList * attributeList)
{
    ((OTF2Importer *) userData)->MPILocations.insert(locationID);

    unsigned long location = ((OTF2Importer *) userData)->locationIndexMap->at(locationID);
    uint64_t converted_time = convertTime(userData, time);
    ((OTF2Importer *) userData)->collective_begins->at(location)->push_back(converted_time);
    return OTF2_CALLBACK_SUCCESS;
}

OTF2_CallbackCode OTF2Importer::callbackMPICollectiveEnd(OTF2_LocationRef locationID,
                                                         OTF2_TimeStamp time,
                                                         void * userData,
                                                         OTF2_AttributeList * attributeList,
                                                         OTF2_CollectiveOp collectiveOp,
                                                         OTF2_CommRef communicator,
                                                         uint32_t root,
                                                         uint64_t sizeSent,
                                                         uint64_t sizeReceived)
{
    ((OTF2Importer *) userData)->MPILocations.insert(locationID);

    unsigned long location = ((OTF2Importer *) userData)->locationIndexMap->at(locationID);

    ((OTF2Importer *) userData)->collective_fragments->at(location)->push_back(new OTF2CollectiveFragment(convertTime(userData, time),
                                                                                                       collectiveOp,
                                                                                                       communicator,
                                                                                                       root));
    return OTF2_CALLBACK_SUCCESS;
}

void OTF2Importer::processCollectives()
{
    int id = 0;

    // Have to check each process in case of single process communicator
    // But probably later processes will not have many fragments left
    // after processing by earlier fragments
    for (int i = 0; i < num_processes; i++)
    {
        std::list<OTF2CollectiveFragment *> * fragments = collective_fragments->at(i);
        while (!fragments->empty())
        {
            // Unmatched as of yet fragment becomes a CollectiveRecord
            OTF2CollectiveFragment * fragment = fragments->front();
            CollectiveRecord * cr = new CollectiveRecord(id, fragment->root,
                                                         fragment->op,
                                                         commIndexMap->at(fragment->comm));
            collectives->insert(std::pair<unsigned long long, CollectiveRecord *>(id, cr));

            // Look through fragment list of other members of communicator for
            // matching fragments
            std::vector<uint64_t> * members = groupMap->at(commMap->at(fragment->comm)->group)->members;
            for (std::vector<uint64_t>::iterator process = members->begin();
                 process != members->end(); ++process)
            {
                OTF2CollectiveFragment * match = NULL;
                for (std::list<OTF2CollectiveFragment *>::iterator cf
                     = collective_fragments->at(*process)->begin();
                     cf != collective_fragments->at(*process)->end(); ++cf)
                {
                    // A match!
                    if ((*cf)->op == fragment->op
                        && (*cf)->comm == fragment->comm
                        && (*cf)->root == fragment->root)
                    {
                        match = *cf;
                        break;
                    }
                }

                if (!match)
                {
                    std::cout << "Error, no matching collective found for";
                    std::cout << " collective type " << int(fragment->op);
                    std::cout << " on communicator ";
                    std::cout << stringMap->at(commMap->at(fragment->comm)->name).c_str();
                    std::cout << " for process " << *process << std::endl;
                }
                else
                {
                    // It's kind of weird that I can't expect the fragments to be in order
                    // but I have to rely on the begin_times being in order... we'll see
                    // if they actually work out.
                    uint64_t begin_time = collective_begins->at(*process)->front();
                    collective_begins->at(*process)->pop_front();
                    std::list<OTF2CollectiveFragment *>::iterator delit
                            = std::find(collective_fragments->at(*process)->begin(),
                                        collective_fragments->at(*process)->end(),
                                        match);
                    if (delit != collective_fragments->at(*process)->end())
                        collective_fragments->at(*process)->erase(delit);

                    collectiveMap->at(*process)->insert(std::pair<unsigned long long, CollectiveRecord *>(begin_time, cr));
                    rawtrace->collectiveBits->at(*process)->push_back(new RawTrace::CollectiveBit(begin_time, cr));
                }
            }

            id++;
        }
    }
}
