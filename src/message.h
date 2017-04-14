#ifndef MESSAGE_H
#define MESSAGE_H

class P2PEvent;
class CommEvent;

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

    bool operator<(const Message &);
    bool operator>(const Message &);
    bool operator<=(const Message &);
    bool operator>=(const Message &);
    bool operator==(const Message &);
};

#endif // MESSAGE_H
