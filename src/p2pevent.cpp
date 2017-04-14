#include "p2pevent.h"
#include "message.h"
#include "metrics.h"
#include <iostream>

P2PEvent::P2PEvent(unsigned long long _enter, unsigned long long _exit,
                   int _function, int _entity, int _pe, int _phase,
                   std::vector<Message *> *_messages)
    : CommEvent(_enter, _exit, _function, _entity, _pe, _phase),
      messages(_messages),
      is_recv(false)
{
}

P2PEvent::~P2PEvent()
{
    for (std::vector<Message *>::iterator itr = messages->begin();
         itr != messages->end(); ++itr)
    {
            delete *itr;
            *itr = NULL;
    }
    delete messages;
}

bool P2PEvent::operator<(const P2PEvent &event)
{
    return enter < event.enter;
}

bool P2PEvent::operator>(const P2PEvent &event)
{
    return enter > event.enter;
}

bool P2PEvent::operator<=(const P2PEvent &event)
{
    return enter <= event.enter;
}

bool P2PEvent::operator>=(const P2PEvent &event)
{
    return enter >= event.enter;
}

bool P2PEvent::operator==(const P2PEvent &event)
{
    return enter == event.enter
            && is_recv == event.is_recv;
}


bool P2PEvent::isReceive() const
{
    return is_recv;
}
