#ifndef ENTITYGROUP_H
#define ENTITYGROUP_H

#include <string>
#include <vector>
#include <map>

// This class is for sub-groupings and reorderings of existing PrimaryEntityGroups.
class EntityGroup
{
public:
    EntityGroup(int _id, std::string _name);
    ~EntityGroup() { delete entities; delete entityorder; }

    int id;
    std::string name;
    // May need to keep additional names if we have several that are the same

    // This order is important for communicator rank ID
    std::vector<unsigned long long> * entities;
    std::map<unsigned long, int> * entityorder;
};

#endif // ENTITYGROUP_H
