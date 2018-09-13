#ifndef GUIDRECORD_H
#define GUIDRECORD_H

class Message;

// For message matching.
class GUIDRecord
{
public:
    GUIDRecord(unsigned long _s, unsigned long long int _st,
               unsigned long _r, unsigned long long int _rt);

    unsigned long parent;
    unsigned long long int parent_time;
    unsigned long child;
    unsigned long long int child_time;

    bool matched;

    Message * message;

    bool operator<(const  GUIDRecord &);
    bool operator>(const  GUIDRecord &);
    bool operator<=(const  GUIDRecord &);
    bool operator>=(const  GUIDRecord &);
    bool operator==(const  GUIDRecord &);
};

#endif // GUIDRECORD_H
