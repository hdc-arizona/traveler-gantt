#ifndef VISWIDGET_H
#define VISWIDGET_H

#include <QGLWidget>
#include <QVector>
#include <climits>

#include "trace.h"

class VisWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit VisWidget(QWidget *parent = 0);
    ~VisWidget();
    virtual void setTrace(Trace *t);
    virtual void processVis();
    QSize sizeHint() const;

signals:
    void repaintAll();
    void stepsChanged(float start, float stop);


public slots:
    virtual void setSteps(float start, float stop);

protected:
    void initializeGL();
    void paintEvent(QPaintEvent *event);

    virtual void drawNativeGL();
    virtual void qtPaint(QPainter *painter);

private:
    void beginNativeGL();
    void endNativeGL();

protected:
    Trace * trace;
    bool visProcessed;
    QColor backgroundColor;
    QBrush selectColor;
    bool changeSource;
};

#endif // VISWIDGET_H
