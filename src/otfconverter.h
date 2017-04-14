#ifndef OTFCONVERTER_H
#define OTFCONVERTER_H

#include <string>
#include <map>
#include <stack>

class RawTrace;
class OTFImporter;
class OTF2Importer;
class Trace;
class CommEvent;
class CounterRecord;
class EventRecord;

// Uses the raw records read from the OTF:
// - switches point events into durational events
// - builds call tree
// - matches messages to durational events
class OTFConverter
{
public:
    OTFConverter();
    ~OTFConverter();

    Trace * importOTF(std::string filename);
    Trace * importOTF2(std::string filename);

signals:
    void finishRead();
    void matchingUpdate(int, std::string);

private:
    void convert();
    void matchEvents();
    void matchEventsSaved();
    void makeSingletonPartition(CommEvent * evt);
    void addToSavedPartition(CommEvent * evt, int partition);
    void handleSavedAttributes(CommEvent * evt, EventRecord *er);
    int advanceCounters(CommEvent * evt, std::stack<CounterRecord *> * counterstack,
                        std::vector<CounterRecord *> * counters, int index,
                        QMstd::map<unsigned int, CounterRecord *> * lastcounters);

    RawTrace * rawtrace;
    Trace * trace;

    static const int event_match_portion = 24;
    static const int message_match_portion = 0;
    static const std::string collectives_string;

};

#endif // OTFCONVERTER_H
