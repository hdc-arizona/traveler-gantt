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

    Trace * doImportOTF(std::string dataFileName, bool logging);
    Trace *doImportOTF2(std::string dataFileName, bool logging);

private:
    Trace * trace;
};

#endif // IMPORTFUNCTOR_H
