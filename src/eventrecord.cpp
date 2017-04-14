#include "eventrecord.h"
#include "event.h"

EventRecord::EventRecord(unsigned long _entity, unsigned long long int _t,
                         unsigned int _v, bool _e)
    : entity(_entity),
      time(_t),
      value(_v),
      enter(_e),
      children(std::vector<Event *>()),
      metrics(NULL)
{
}

EventRecord::~EventRecord()
{
    if (metrics)
        delete metrics;
}

bool EventRecord::operator<(const EventRecord &event)
{
    return enter < event.enter;
}

bool EventRecord::operator>(const EventRecord &event)
{
    return enter > event.enter;
}

bool EventRecord::operator<=(const EventRecord &event)
{
    return enter <= event.enter;
}

bool EventRecord::operator>=(const EventRecord &event)
{
    return enter >= event.enter;
}

bool EventRecord::operator==(const EventRecord &event)
{
    return enter == event.enter;
}
