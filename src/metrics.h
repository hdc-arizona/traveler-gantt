#ifndef METRICS_H
#define METRICS_H

#include <QMap>
#include <QString>
#include <QList>

class Metrics
{
public:
    Metrics();
    ~Metrics();
    void addMetric(QString name, double event_value);
    void setMetric(QString name, double event_value);
    bool hasMetric(QString name);
    double getMetric(QString name);
    QList<QString> getMetricList();

    class MetricPair {
    public:
        MetricPair(double _e)
            : event(_e) {}

        double event; // value at event
    };

    QMap<QString, MetricPair *> * metrics; // Lateness or Counters etc
};

#endif // METRICS_H
