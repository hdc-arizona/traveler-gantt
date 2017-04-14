#ifndef P2PEVENT_H
#define P2PEVENT_H

#include "commevent.h"

class P2PEvent : public CommEvent
{
public:
    P2PEvent(unsigned long long _enter, unsigned long long _exit,
             int _function, int _entity, int _pe, int _phase,
             std::vector<Message *> * _messages = NULL);
    ~P2PEvent();

    // Based on enter time, add_order & receive-ness
    bool operator<(const P2PEvent &);
    bool operator>(const P2PEvent &);
    bool operator<=(const P2PEvent &);
    bool operator>=(const P2PEvent &);
    bool operator==(const P2PEvent &);


    int comm_count(std::map<Event *, int> *memo = NULL) { return 1; }
    bool isP2P() { return true; }
    bool isReceive() const;

    CommEvent * compare_to_sender(CommEvent * prev);

    std::vector<Message *> * getMessages() { return messages; }


    // Messages involved wiht this event
    std::vector<Message *> * messages;

    bool is_recv;
};

#endif // P2PEVENT_H
