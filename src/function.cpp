#include "function.h"

Function::Function(std::string _n, int _g, std::string _s, int _c)
    : name(_n),
      shortname(_s),
      group(_g),
      comms(_c),
      count(0),
      rank(0),
      isMain(false),
      task_lengths(std::vector<unsigned long long>())
{
}


// These are just for visualization purposes, NOT serialization purposes
void to_json(json& j, const Function& f)
{
    j = json{
        {"name", f.name},
        {"shortname", f.shortname},
        {"count", std::to_string(f.count)},
        {"rank", std::to_string(f.rank)}
    };
}

void from_json(const json& j, Function& f)
{
    f.name = j.at("name").get<std::string>();
    f.shortname = j.at("shortname").get<std::string>();
    f.count = std::stoull(j.at("count").get<std::string>());
    f.rank = j.at("rank").get<int>();
}

void to_json(json& j, const Function * f)
{
    j = json{
        {"name", f->name},
        {"shortname", f->shortname},
        {"count", std::to_string(f->count)},
        {"rank", std::to_string(f->rank)}
    };
}

void from_json(const json& j, Function * f)
{
    f->name = j.at("name").get<std::string>();
    f->shortname = j.at("shortname").get<std::string>();
    f->count = std::stoull(j.at("count").get<std::string>());
    f->rank = j.at("rank").get<int>();
}
