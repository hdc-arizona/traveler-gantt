#ifndef MULTIEVENT_H
#define MULTIEVENT_H

#include "p2pevent.h"
#include <vector>

// Collection of individual events
// e.g., suspended/restarted/futures in HPX
class MultiEvent 
{
public:
    MultiEvent(uint64_t _guid);
    ~MultiEvent();

    // Based on enter time, add_order & receive-ness
    bool operator<(const MultiEvent &);
    bool operator>(const MultiEvent &);
    bool operator<=(const MultiEvent &);
    bool operator>=(const MultiEvent &);
    bool operator==(const MultiEvent &);

    std::vector<P2PEvent *> * getEvents() { return events; }

    uint64_t guid;

    // Events that make up this multi-part event
    std::vector<P2PEvent *> * events;
};

#endif // MULTIEVENT_H
