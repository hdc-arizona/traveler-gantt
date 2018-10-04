#ifndef MULTIRECORD_H
#define MULTIRECORD_H

#include <vector>

class EventRecord;
class GUIDRecord;

// For message matching.
class MultiRecord
{
public:
    MultiRecord(uint64_t _guid);
    ~MultiRecord();      

    uint64_t guid;

    std::vector<EventRecord *> * events;
    std::vector<GUIDRecord *> * to_crs;

    bool operator<(const  MultiRecord &);
    bool operator>(const  MultiRecord &);
    bool operator<=(const  MultiRecord &);
    bool operator>=(const  MultiRecord &);
    bool operator==(const  MultiRecord &);
};

#endif // MULTIRECORD_H
