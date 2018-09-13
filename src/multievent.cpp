#include "multievent.h"
#include <iostream>

MultiEvent::MultiEvent(uint64_t _guid)
    : guid(_guid),
      events(new std::vector<P2PEvent *>())
{

}

MultiEvent::~MultiEvent()
{
    delete events;
}

bool MultiEvent::operator<(const MultiEvent &event)
{
    return guid < event.guid;
}

bool MultiEvent::operator>(const MultiEvent &event)
{
    return guid > event.guid;
}

bool MultiEvent::operator<=(const MultiEvent &event)
{
    return guid <= event.guid;
}

bool MultiEvent::operator>=(const MultiEvent &event)
{
    return guid >= event.guid;
}

bool MultiEvent::operator==(const MultiEvent &event)
{
    return guid == event.guid;
}

