#ifndef FUNCTION_H
#define FUNCTION_H

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Information about function calls from OTF
class Function
{
public:
    Function(std::string _n, int _g, std::string _s = "", int _c = 0);

    std::string name;
    std::string shortname;
    int group;
    int comms; // max comms in a function
    unsigned long long count; // number of times it appears in trace
    int rank; // how it comes to other functions in terms of use
    bool isMain;



    static bool functionCountGreaterThan(const Function * f1, const Function * f2)
    {
        return f1->count > f2->count;
    }
};

// For visualization purposes, NOT serialization purposes
void to_json(json& j, const Function& f);
void from_json(const json& j, Function& f);
void to_json(json& j, const Function * f);
void from_json(const json& j, Function * f);

#endif // FUNCTION_H
