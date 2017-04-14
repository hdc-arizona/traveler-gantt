#include "metrics.h"

Metrics::Metrics()
    : metrics(new std::map<std::string, double>())
{
}

Metrics::~Metrics()
{
    delete metrics;
}

void Metrics::addMetric(std::string name, double event_value)
{
    (*metrics)[name] = event_value;
}

void Metrics::setMetric(std::string name, double event_value)
{
    (*metrics)[name] = event_value;
}

bool Metrics::hasMetric(std::string name)
{
    return metrics->contains(name);
}

double Metrics::getMetric(std::string name)
{
    return ((*metrics)[name]);
}

std::vector<std::string> Metrics::getMetricList()
{
    std::vector<std::string> names = std::vector<std::string>();
    for (std::map<std::string, double>::iterator counter = metrics->begin();
         counter != metrics->end(); ++counter)
    {
        names.append(counter.key());
    }
    return names;
}
