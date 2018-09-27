#include "multirecord.h"
#include "eventrecord.h"
#include <cstdlib>

MultiRecord::MultiRecord(uint64_t _guid) :
    guid(_guid),
    events(new std::vector<EventRecord *>())
{
}

MultiRecord::~MultiRecord()
{
    delete events;
}

bool  MultiRecord::operator<(const  MultiRecord & cr)
{
    return guid <  cr.guid;
}

bool  MultiRecord::operator>(const  MultiRecord & cr)
{
    return guid >  cr.guid;
}

bool  MultiRecord::operator<=(const  MultiRecord & cr)
{
    return guid <=  cr.guid;
}

bool  MultiRecord::operator>=(const  MultiRecord & cr)
{
    return guid >=  cr.guid;
}

bool  MultiRecord::operator==(const  MultiRecord & cr)
{
    return guid ==  cr.guid;
}

