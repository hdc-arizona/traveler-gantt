#ifndef METRICS_H
#define METRICS_H

#include <map>
#include <string>
#include <vector>

class Metrics
{
public:
    Metrics();
    ~Metrics();
    void addMetric(std::string name, double event_value);
    void setMetric(std::string name, double event_value);
    bool hasMetric(std::string name);
    double getMetric(std::string name);
    std::vector<std::string> getMetricList();

    std::map<std::string, double> * metrics; // Lateness or Counters etc
};

#endif // METRICS_H
