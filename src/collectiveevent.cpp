#include "collectiveevent.h"

CollectiveEvent::CollectiveEvent(unsigned long long _enter,
                                 unsigned long long _exit,
                                 int _function, int _entity, int _pe, int _phase,
                                 CollectiveRecord *_collective)
    : CommEvent(_enter, _exit, _function, _entity, _pe, _phase),
      collective(_collective)
{
}

CollectiveEvent::~CollectiveEvent()
{
    delete collective;
}
