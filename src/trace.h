#ifndef TRACE_H
#define TRACE_H

#include <string>
#include <map>
#include <vector>
#include <queue>
#include <stack>
#include <memory>
#include <ctime>
#include <stdint.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Event;
class Message;
class CommEvent;
class Function;
class EntityGroup;
class PrimaryEntityGroup;
class OTFCollective;
class CollectiveRecord;

class Trace
{
public:
    Trace(int nt, int np);
    ~Trace();

    void preprocess();
    Event * findEvent(int entity, unsigned long long time);
    json timeToJSON(unsigned long long start, unsigned long long stop,
                    unsigned long long entity_start,
                    unsigned long long entities,
                    unsigned long width,
                    unsigned long long taskid,
                    unsigned long long task_time,
                    bool logging);
    json initJSON(unsigned long width, bool logging);
    json timeOverview(unsigned long width, bool logging);
    std::string name;
    std::string fullpath;
    int num_entities;
    int num_pes;
    int units;
    int max_depth;
    uint64_t totalTime;

    std::vector<std::string> * metrics;
    std::map<std::string, std::string> * metric_units;

    // Below set by OTFConverter
    std::map<int, std::string> * functionGroups;
    std::map<int, Function *> * functions;

    std::map<int, PrimaryEntityGroup *> * primaries;
    PrimaryEntityGroup * processingElements;
    std::map<int, EntityGroup *> * entitygroups;
    std::map<int, OTFCollective *> * collective_definitions;

    std::map<unsigned long long, CollectiveRecord *> * collectives;
    std::vector<std::map<unsigned long long, CollectiveRecord *> *> * collectiveMap;

    std::vector<std::vector<Event *> *> * events; // This is going to be by entities
    std::vector<std::vector<Event *> *> * roots; // Roots of call trees per pe

    int mpi_group; // functionGroup index of "MPI" functions

    unsigned long long max_time; // largest time
    unsigned long long min_time; // starting time
    unsigned long long last_init; // time at which MPI_Init stops
    unsigned long long last_finalize; // time at which last Finalize -starts-

private:
    bool isProcessed; // Partitions exist
    void timeEventToJSON(Event * evt, int depth,
                         unsigned long long start, unsigned long long stop,
                         unsigned long long entity_start,
                         unsigned long long entities,
                         unsigned long long min_span,
                         unsigned long long taskid,
                         std::vector<json>& slice,
                         std::vector<json>& msg_slice,
                         std::vector<json>& collective_slice,
                         std::vector<std::vector<json> >& parent_slice,
                         std::map<std::string, std::string>& function_names);
    void eventTraceBackJSON(Event * evt, unsigned long long start,
                            unsigned long long stop, 
                            unsigned long long entity_start, unsigned long long entities,
                            unsigned long long min_span, 
                            unsigned long long taskid, unsigned long long task_time,
                            std::vector<json>& msg_slice, 
                            bool logging);
    void msgTraceBackJSON(CommEvent * evt, int depth, bool sibling, Message * last,
                          unsigned long long start, unsigned long long stop, 
                          unsigned long long entity_start, unsigned long long entities,
                          unsigned long long min_span, 
                          std::vector<json>&msg_slice,
                          bool logging);
    static const bool debug = false;
    static const int partition_portion = 25;
    static const int lateness_portion = 45;
    static const int steps_portion = 30;
    static const std::string collectives_string;
};

#endif // TRACE_H
