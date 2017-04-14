#include "message.h"
#include "commevent.h"
#include "p2pevent.h"

Message::Message(unsigned long long send, unsigned long long recv, int group)
    : sender(NULL), receiver(NULL),
      sendtime(send), recvtime(recv), entitygroup(group), tag(0)
{
}

bool Message::operator<(const Message &message)
{
    return sendtime < message.sendtime;
}

bool Message::operator>(const Message &message)
{
    return sendtime > message.sendtime;
}

bool Message::operator<=(const Message &message)
{
    return sendtime <= message.sendtime;
}

bool Message::operator>=(const Message &message)
{
    return sendtime >= message.sendtime;
}

bool Message::operator==(const Message &message)
{
    return sendtime == message.sendtime;
}


CommEvent * Message::getDesignee()
{
    return sender;
}
