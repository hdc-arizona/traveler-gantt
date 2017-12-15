#include "message.h"
#include "commevent.h"
#include "p2pevent.h"

Message::Message(unsigned long long send, unsigned long long recv, int group)
    : sender(NULL), receiver(NULL),
      sendtime(send), recvtime(recv), entitygroup(group), tag(0), id(0)
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

/*
void to_json(json& j, const Message& m)
{
    j = json{
        {"id", m.id},
        {"sendtime", m.sendtime},
        {"recvtime", m.recvtime},
        {"sender_entity", m.sender->entity},
        {"receiver_entity", m.receiver->entity}
    };
}
*/

/* commented pre-rostam
void from_json(const json& j, Message& m)
{
    m.id = j.at("id").get<unsigned long long>();
    m.sendtime = j.at("sendtime").get<unsigned long long>();
    m.recvtime = j.at("recvtime").get<unsigned long long>();
//    m.sender = j.at("sender").get<unsigned long long>();
//    m.receiver = j.at("receiver").get<unsigned long long>();
}
*/

/*
void to_json(json& j, const Message * m)
{
    j = json{
        {"id", m->id},
        {"sendtime", m->sendtime},
        {"recvtime", m->recvtime},
        {"sender_entity", m->sender->entity},
        {"receiver_entity", m->receiver->entity}
    };
}
*/

/* commented pre-rostam
void from_json(const json& j, Message * m)
{
    m->id = j.at("id").get<unsigned long long>();
    m->sendtime = j.at("sendtime").get<unsigned long long>();
    m->recvtime = j.at("recvtime").get<unsigned long long>();
//    m->sender = j.at("sender").get<unsigned long long>();
//    m->receiver = j.at("receiver").get<unsigned long long>();
}
*/

