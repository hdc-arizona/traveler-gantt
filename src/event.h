#ifndef EVENT_H
#define EVENT_H

#include <vector>
#include <string>
#include <otf2/otf2.h>
#include "json.hpp"

class Function;
class Metrics;

using json = nlohmann::json;

class Event
{
public:
    Event(unsigned long long _enter, unsigned long long _exit, int _function,
          unsigned long _entity, unsigned long _pe);
    ~Event();

    // Based on enter time
    bool operator<(const Event &);
    bool operator>(const Event &);
    bool operator<=(const Event &);
    bool operator>=(const Event &);
    bool operator==(const Event &);
    static bool eventEntityLessThan(const Event * evt1, const Event * evt2)
    {
        return evt1->entity < evt2->entity;
    }

    Event * findChild(unsigned long long time);
    unsigned long long getVisibleEnd(unsigned long long start);
    virtual bool isCommEvent() { return false; }
    virtual bool isReceive() const { return false; }
    virtual bool isCollective() { return false; }

    bool hasMetric(std::string name);
    double getMetric(std::string name);
    void addMetric(std::string name, double event_value);

    // Call tree info
    Event * caller;
    std::vector<Event *> * callees;

    unsigned long long enter;
    unsigned long long exit;
    int function;
    unsigned long entity;
    unsigned long pe;
    int depth;

    Metrics * metrics; // Lateness or Counters etc
};

void to_json(json& j, const Event& e)
{
    j = json{
        {"enter", e.enter},
        {"exit", e.exit},
        {"function", e.function},
        {"entity", e.entity}
    };
}

void from_json(const json& j, Event& e)
{
    e.enter = j.at("enter").get<unsigned long long>();
    e.exit = j.at("exit").get<unsigned long long>();
    e.function = j.at("function").get<int>();
    e.entity = j.at("entity").get<unsigned long>();
}


#endif // EVENT_H
