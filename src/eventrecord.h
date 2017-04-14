#ifndef EVENTRECORD_H
#define EVENTRECORD_H

#include <vector>
#include <string>
#include <map>

class Event;

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

    // Based on time
    bool operator<(const EventRecord &);
    bool operator>(const EventRecord &);
    bool operator<=(const EventRecord &);
    bool operator>=(const EventRecord &);
    bool operator==(const EventRecord &);
};

#endif // EVENTRECORD_H
