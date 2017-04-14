#ifndef PRIMARYENTITYGROUP_H
#define PRIMARYENTITYGROUP_H

#include <vector>
#include <string>

class Entity;

// This is the main class for organizing entities.
// In MPI traces, we'll only have one (essentially MPI_COMM_WORLD)
// but in something like Charm++ we'll have many
class PrimaryEntityGroup
{
public:
    PrimaryEntityGroup(int _id, std::string _name);
    ~PrimaryEntityGroup();

    int id;
    std::string name;

    // This order is important for communicator rank ID
    std::vector<Entity *> * entities;
};

#endif // PRIMARYENTITYGROUP_H
