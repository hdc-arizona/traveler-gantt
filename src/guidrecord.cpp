#include "guidrecord.h"
#include "message.h"
#include <cstdlib>

GUIDRecord::GUIDRecord(unsigned long _s, unsigned long long int _st,
                       unsigned long _r, unsigned long long int _rt) :
    parent(_s), parent_time(_st), child(_r), child_time(_rt),
    matched(true),
    message(NULL)
{
}


bool  GUIDRecord::operator<(const  GUIDRecord & cr)
{
    return parent_time <  cr.parent_time;
}

bool  GUIDRecord::operator>(const  GUIDRecord & cr)
{
    return parent_time >  cr.parent_time;
}

bool  GUIDRecord::operator<=(const  GUIDRecord & cr)
{
    return parent_time <=  cr.parent_time;
}

bool  GUIDRecord::operator>=(const  GUIDRecord & cr)
{
    return parent_time >=  cr.parent_time;
}

bool  GUIDRecord::operator==(const  GUIDRecord & cr)
{
    return parent_time ==  cr.parent_time;
}

