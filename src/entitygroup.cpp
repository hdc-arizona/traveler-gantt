#include "entitygroup.h"

EntityGroup::EntityGroup(int _id, std::string _name)
    : id(_id),
      name(_name),
      entities(new std::vector<unsigned long long>()),
      entityorder(new std::map<unsigned long, int>())
{
}
