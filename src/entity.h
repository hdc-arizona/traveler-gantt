#ifndef ENTITY_H
#define ENTITY_H

#include <string>

class PrimaryEntityGroup;

class Entity
{
public:
    Entity(unsigned long _id, std::string _name, PrimaryEntityGroup * _primary);

    unsigned long id;
    std::string name;

    PrimaryEntityGroup * primary;
};

#endif // ENTITY_H
