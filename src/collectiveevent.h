#ifndef COLLECTIVEEVENT_H
#define COLLECTIVEEVENT_H

#include "commevent.h"
#include "collectiverecord.h"

class CollectiveEvent : public CommEvent
{
public:
    CollectiveEvent(unsigned long long _enter, unsigned long long _exit,
                    int _function, int _entity, int _pe, int _phase,
                    CollectiveRecord * _collective);
    ~CollectiveEvent();

    // We count the collective as two since it serves as both the beginning
    // and ending of some sort of communication while P2P communication
    // events are either the begin (send) or the end (recv)
    int comm_count(std::map<Event *, int> *memo = NULL) { return 2; }

    bool isP2P() { return false; }
    bool isReceive() { return false; }
    virtual bool isCollective() { return true; }

    CollectiveRecord * getCollective() { return collective; }

    CollectiveRecord * collective;

private:
    void set_stride_relationships();
};

#endif // COLLECTIVEEVENT_H
