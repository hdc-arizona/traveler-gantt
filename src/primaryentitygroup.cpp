#include "primaryentitygroup.h"
#include "entity.h"

PrimaryEntityGroup::PrimaryEntityGroup(int _id, std::string _name)
    : id(_id),
      name(_name),
      entities(new std::vector<Entity *>())
{
}

PrimaryEntityGroup::~PrimaryEntityGroup()
{
    for (std::vector<Entity *>::iterator entity = entities->begin();
         entity != entities->end(); ++entity)
    {
        delete *entity;
        *entity = NULL;
    }
    delete entities;
}
