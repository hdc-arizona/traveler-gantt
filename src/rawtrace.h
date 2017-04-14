#ifndef RAWTRACE_H
#define RAWTRACE_H

#include <string>
#include <map>
#include <vector>
#include <stdint.h>

class PrimaryEntityGroup;
class EntityGroup;
class CommRecord;
class OTFCollective;
class CollectiveRecord;
class Function;
class Counter;
class CounterRecord;
class EventRecord;

// Trace from OTF without processing
class RawTrace
{
public:
    RawTrace(int nt, int np);
    ~RawTrace();

    class CollectiveBit {
    public:
        CollectiveBit(uint64_t _time, CollectiveRecord * _cr)
            : time(_time), cr(_cr) {}

        uint64_t time;
        CollectiveRecord * cr;
    };

    std::map<int, PrimaryEntityGroup *> * primaries;
    PrimaryEntityGroup * processingElements;
    std::map<int, std::string> * functionGroups;
    std::map<int, Function *> * functions;
    std::vector<std::vector<EventRecord *> *> * events;
    std::vector<std::vector<CommRecord *> *> * messages;
    std::vector<std::vector<CommRecord *> *> * messages_r; // by receiver instead of sender
    std::map<int, EntityGroup *> * entitygroups;
    std::map<int, OTFCollective *> * collective_definitions;
    std::map<unsigned int, Counter *> * counters;
    std::vector<std::vector<CounterRecord * > *> * counter_records;

    std::map<unsigned long long, CollectiveRecord *> * collectives;
    std::vector<std::map<unsigned long long, CollectiveRecord *> *> * collectiveMap;
    std::vector<std::vector<CollectiveBit *> *> * collectiveBits;
    int num_entities;
    int num_pes;
    int second_magnitude; // seconds are 10^this over the smallest smaple unit
    std::vector<std::string> * metric_names;
    std::map<std::string, std::string> * metric_units;
};

#endif // RAWTRACE_H
