#include "entity.h"

Entity::Entity(unsigned long _id, std::string _name, PrimaryEntityGroup *_primary)
    : id(_id),
      name(_name),
      primary(_primary)
{
}
