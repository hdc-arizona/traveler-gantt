#ifndef STEPVIS_H
#define STEPVIS_H

#include "timelinevis.h"

class StepVis : public TimelineVis
{
    Q_OBJECT
public:
    StepVis(QWidget* parent = 0);
    ~StepVis();
    void setTrace(Trace * t);

    void mouseMoveEvent(QMouseEvent * event);
    void wheelEvent(QWheelEvent * event);

public slots:
    void setSteps(float start, float stop);

protected:
    void qtPaint(QPainter *painter);
    void drawNativeGL();
    void paintEvents(QPainter *painter);
    void prepaint();
    void drawColorBarGL();
    void drawColorBarText(QPainter * painter);

private:
    bool showAggSteps;
    long long maxLateness;
    ColorMap * colormap;
    QString maxLatenessText;
    int maxLatenessTextWidth;

    static const int spacingMinimum = 12;
    static const int colorBarHeight = 20;
};

#endif // STEPVIS_H
