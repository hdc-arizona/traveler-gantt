#ifndef FUNCTION_H
#define FUNCTION_H

#include <string>

// Information about function calls from OTF
class Function
{
public:
    Function(std::string _n, int _g, std::string _s = "", int _c = 0);

    std::string name;
    std::string shortname;
    int group;
    int comms; // max comms in a function
    bool isMain;
};

#endif // FUNCTION_H
