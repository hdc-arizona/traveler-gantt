#ifndef IMPORTFUNCTOR_H
#define IMPORTFUNCTOR_H

#include <string>

class Trace;

// Handle signaling for progress bar
class ImportFunctor
{
public:
    ImportFunctor();
    Trace * getTrace() { return trace; }

public slots:
    void doImportOTF(std::string dataFileName);
    void doImportOTF2(std::string dataFileName);
    void finishInitialRead();
    void updateMatching(int portion, std::string msg);
    void updatePreprocess(int portion, std::string msg);
    void switchProgress();

signals:
    void switching();
    void done(Trace *);
    void reportProgress(int, std::string);

private:
    Trace * trace;
};

#endif // IMPORTFUNCTOR_H
