#ifndef EVENTRECORD_H
#define EVENTRECORD_H

#include <vector>
#include <string>
#include <map>

class Event;
class GUIDRecord;

// Holder for OTF Event info
class EventRecord
{
public:
    EventRecord(unsigned long _entity, unsigned long long int _t, unsigned int _v, bool _e = true);
    ~EventRecord();

    unsigned long entity;
    unsigned long long int time;
    unsigned int value;
    bool enter;
    std::vector<Event *> children;
    std::map<std::string, unsigned long long> * metrics;

    uint64_t guid;
    uint64_t parent_guid;
    void setGUID(uint64_t g) { guid = g; };
    uint64_t getGUID() { return guid; };
    void setParentGUID(uint64_t g) { parent_guid = g; };
    uint64_t getParentGUID() { return parent_guid; };
    GUIDRecord * from_cr;
    std::vector<GUIDRecord *> * to_crs;
    void setFromGUIDRecord(GUIDRecord * _cr) { from_cr = _cr; };
    GUIDRecord * getFromGUIDRecord() { return from_cr; };

    // Based on time
    bool operator<(const EventRecord &);
    bool operator>(const EventRecord &);
    bool operator<=(const EventRecord &);
    bool operator>=(const EventRecord &);
    bool operator==(const EventRecord &);
};

#endif // EVENTRECORD_H
