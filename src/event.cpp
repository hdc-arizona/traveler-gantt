#include "event.h"
#include "function.h"
#include "metrics.h"
#include <iostream>

Event::Event(unsigned long long _enter, unsigned long long _exit,
             int _function, unsigned long _entity, unsigned long _pe)
    : id(0),
      guid(0),
      parent_guid(0),
      caller(NULL),
      callees(new std::vector<Event *>()),
      enter(_enter),
      exit(_exit),
      function(_function),
      entity(_entity),
      pe(_pe),
      depth(-1),
      metrics(new Metrics())
{

}

Event::~Event()
{
    delete metrics;
    if (callees)
        delete callees;
}

bool Event::operator<(const Event &event)
{
    if (enter == event.enter)
    {
        if (this->isReceive())
            return true;
        else
            return false;
    }
    return enter < event.enter;
}

bool Event::operator>(const Event &event)
{
    if (enter == event.enter)
    {
        if (this->isReceive())
            return false;
        else
            return true;
    }
    return enter > event.enter;
}

bool Event::operator<=(const Event &event)
{
    return enter <= event.enter;
}

bool Event::operator>=(const Event &event)
{
    return enter >= event.enter;
}

bool Event::operator==(const Event &event)
{
    return enter == event.enter;
}

Event * Event::findChild(unsigned long long time)
{
    Event * result = NULL;
    Event * child_match = NULL;
    if (enter <= time && exit >= time)
    {
        result = this;
        for (std::vector<Event *>::iterator child = callees->begin();
             child != callees->end(); ++child)
        {
            child_match = (*child)->findChild(time);
            if (child_match)
            {
                result = child_match;
                break;
            }
        }
    }
    return result;
}

unsigned long long Event::getVisibleEnd(unsigned long long start)
{
    unsigned long long end = exit;
    for (std::vector<Event *>::iterator child = callees->begin();
         child != callees->end(); ++child)
    {
        if ((*child)->enter > start)
        {
            end = (*child)->enter;
            break;
        }
    }
    return end;
}

bool Event::hasMetric(std::string name)
{
    if (metrics->hasMetric(name))
        return true;
    else if (caller && caller->metrics->hasMetric(name))
        return true;
    else
        return false;
}

double Event::getMetric(std::string name)
{
    if (metrics->hasMetric(name))
        return metrics->getMetric(name);

    if (caller && caller->metrics->hasMetric(name))
        return caller->metrics->getMetric(name);

    return 0;
}

void Event::addMetric(std::string name, double event_value)
{
    metrics->addMetric(name, event_value);
}

void to_json(json& j, const Event& e)
{
    j = json{
        {"id", std::to_string(e.id)},
        {"guid", std::to_string(e.guid)},
        {"parent_guid", std::to_string(e.parent_guid)},
        {"enter", e.enter},
        {"exit", e.exit},
        {"function", e.function},
        {"entity", e.entity}
    };
}

void from_json(const json& j, Event& e)
{
    e.id = std::stoull(j.at("id").get<std::string>());
    e.guid = std::stoull(j.at("guid").get<std::string>());
    e.parent_guid = std::stoull(j.at("parent_guid").get<std::string>());
    e.enter = j.at("enter").get<unsigned long long>();
    e.exit = j.at("exit").get<unsigned long long>();
    e.function = j.at("function").get<int>();
    e.entity = j.at("entity").get<unsigned long>();
}

void to_json(json& j, const Event * e)
{
    j = json{
        {"id", std::to_string(e->id)},
        {"guid", std::to_string(e->guid)},
        {"parent_guid", std::to_string(e->parent_guid)},
        {"enter", e->enter},
        {"exit", e->exit},
        {"function", e->function},
        {"entity", e->entity}
    };
}

void from_json(const json& j, Event * e)
{
    e->id = std::stoull(j.at("id").get<std::string>());
    e->guid = std::stoull(j.at("guid").get<std::string>());
    e->parent_guid = std::stoull(j.at("parent_guid").get<std::string>());
    e->enter = j.at("enter").get<unsigned long long>();
    e->exit = j.at("exit").get<unsigned long long>();
    e->function = j.at("function").get<int>();
    e->entity = j.at("entity").get<unsigned long>();
}
