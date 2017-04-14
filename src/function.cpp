#include "function.h"

Function::Function(std::string _n, int _g, std::string _s, int _c)
    : name(_n),
      shortname(_s),
      group(_g),
      comms(_c),
      isMain(false)
{
}
