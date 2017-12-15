#ifndef MESSAGE_H
#define MESSAGE_H

#include <nlohmann/json.hpp>

class P2PEvent;
class CommEvent;

using json = nlohmann::json;

// Holder of message info
class Message
{
public:
    Message(unsigned long long send, unsigned long long recv,
            int group);
    P2PEvent * sender;
    P2PEvent * receiver;
    unsigned long long sendtime;
    unsigned long long recvtime;
    int entitygroup;
    unsigned int tag;
    unsigned long long size;

    CommEvent * getDesignee();

    void setID(unsigned long long i) { id = i; }

    unsigned long long id;

    bool operator<(const Message &);
    bool operator>(const Message &);
    bool operator<=(const Message &);
    bool operator>=(const Message &);
    bool operator==(const Message &);
};

void to_json(json& j, const Message& e);
//void from_json(const json& j, Message& e);
void to_json(json& j, const Message * e);
//void from_json(const json& j, Message * e);

#endif // MESSAGE_H
