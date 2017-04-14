#include "commevent.h"
#include <otf2/OTF2_AttributeList.h>
#include <otf2/OTF2_GeneralDefinitions.h>
#include <iostream>
#include "metrics.h"

CommEvent::CommEvent(unsigned long long _enter, unsigned long long _exit,
                     int _function, int _entity, int _pe, int _phase)
    : Event(_enter, _exit, _function, _entity, _pe),
      comm_next(NULL),
      comm_prev(NULL),
      true_next(NULL),
      true_prev(NULL),
      pe_next(NULL),
      pe_prev(NULL),
      phase(_phase),
      gvid("")
{
}

CommEvent::~CommEvent()
{
}


bool CommEvent::operator<(const CommEvent &event)
{
    return enter < event.enter;
}

bool CommEvent::operator>(const CommEvent &event)
{
    return enter > event.enter;
}

bool CommEvent::operator<=(const CommEvent &event)
{
    return enter <= event.enter;
}

bool CommEvent::operator>=(const CommEvent &event)
{
    return enter >= event.enter;
}

bool CommEvent::operator==(const CommEvent &event)
{
    return enter == event.enter
            && isReceive() == event.isReceive();
}
