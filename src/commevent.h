#ifndef COMMEVENT_H
#define COMMEVENT_H

#include "event.h"
#include <string>
#include <vector>
#include <map>

class Message;
class CollectiveRecord;
class Metrics;

class CommEvent : public Event
{
public:
    CommEvent(unsigned long long _enter, unsigned long long _exit,
              int _function, int _entity, int _pe, int _phase);
    ~CommEvent();

    bool operator<(const CommEvent &);
    bool operator>(const CommEvent &);
    bool operator<=(const CommEvent &);
    bool operator>=(const CommEvent &);
    bool operator==(const CommEvent &);

    virtual int comm_count(std::map<Event *, int> *memo = NULL)=0;
    bool isCommEvent() { return true; }
    virtual bool isP2P() { return false; }
    virtual bool isReceive() const { return false; }
    virtual bool isCollective() { return false; }

    virtual std::vector<Message *> * getMessages() { return NULL; }
    virtual CollectiveRecord * getCollective() { return NULL; }

    CommEvent * comm_next;
    CommEvent * comm_prev;
    CommEvent * true_next;
    CommEvent * true_prev;
    CommEvent * pe_next;
    CommEvent * pe_prev;

    int phase;

    // For graph drawing
    std::string gvid;

};
#endif // COMMEVENT_H
