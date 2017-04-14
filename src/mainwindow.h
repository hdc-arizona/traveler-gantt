#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QStack>
#include <QList>
#include <QVector>

class Event;
class Trace;
class VisWidget;
class VisOptions;
class VisOptionsDialog;

class QAction;
class ImportFunctor;
class OTF2ExportFunctor;
class QProgressDialog;
class QThread;
class QWidget;
class QCloseEvent;

namespace Ui {
class MainWindow;
}

// Mostly GUI handling. Right now also maintains all open traces
// forever, but we need something better to do with that
class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void closeEvent(QCloseEvent *);

public slots:
    void launchVisOptions();

    // Signal relays
    void pushTime(float start, float stop, bool jump = false);
    void selectEvent(Event * event);
    void selectEntities(QList<int> entities);

    // Importing & Progress Bar
    void importTracebyGUI();
    void traceFinished(Trace * trace);
    void updateProgress(int portion, QString msg);

    // High level GUI update
    void handleSplitter(int pos, int index);
    void handleSideSplitter(int pos, int index);
    void togglePhysicalTime();
    void toggleMetricOverview();

    // Change Traces
    void traceTriggered(QAction * action);
    void closeTrace();

    // Settings
    void writeSettings();
    void readSettings();

signals:
    void operate(const QString &);
    void exportTrace(Trace *, const QString&, const QString&);
    
private:
    Ui::MainWindow *ui;
    void importTrace(QString dataFileName);
    void activeTraceChanged(bool first = false);
    void linkSideSplitter();
    void linkMainSplitter();
    void setVisWidgetState();

    // Saving traces & vis
    QList<Trace *> traces;
    QVector<VisWidget *> viswidgets;
    QList<QAction *> visactions;
    QVector<int> splitterMap;
    QVector<QAction *> splitterActions;
    int activeTrace;

    // For progress bar
    ImportFunctor * importWorker;
    QThread * importThread;
    QProgressDialog * progress;
    OTF2ExportFunctor * exportWorker;
    QThread * exportThread;

    // Color stuff & other vis options
    VisOptions * visoptions;
    VisOptionsDialog * visdialog;

    QString activetracename;

    QStack<QString> activetraces;
    QString dataDirectory;
    bool otf1Support;

    static const int OVERVIEW = 0;
    static const int TIMEVIS = 1;
};

#endif // MAINWINDOW_H
