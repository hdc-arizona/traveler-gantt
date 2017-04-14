#ifndef COLLECTIVERECORD_H
#define COLLECTIVERECORD_H

#include <vector>

class CommEvent;
class CollectiveEvent;

// Information we get from OTF about collectives
class CollectiveRecord
{
public:
    CollectiveRecord(unsigned long long int _matching, unsigned int _root,
                     unsigned int _collective, unsigned int _entitygroup);

    unsigned long long int matchingId;
    unsigned int root;
    unsigned int collective;
    unsigned int entitygroup;
    bool mark;

    std::vector<CollectiveEvent *> * events;

    CommEvent * getDesignee();
};

#endif // COLLECTIVERECORD_H
