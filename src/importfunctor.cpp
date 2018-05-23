#include "importfunctor.h"
#include "ravelutils.h"

#include "trace.h"
#include "otfconverter.h"
#include "otf2importer.h"
#include <ctime>

ImportFunctor::ImportFunctor()
    : trace(NULL)
{
}

Trace * ImportFunctor::doImportOTF2(std::string dataFileName, bool logging)
{
    std::cout << "Processing " << dataFileName.c_str() << std::endl;
    clock_t start = clock();

    OTFConverter * importer = new OTFConverter();
    Trace* trace = importer->importOTF2(dataFileName, logging);
    delete importer;

    if (trace)
    {
        trace->preprocess();
    }

    clock_t end = clock();
    double traceElapsed = (end - start) / CLOCKS_PER_SEC;
    RavelUtils::gu_printTime(traceElapsed, "Total trace: ");

    return trace;
}

Trace *ImportFunctor::doImportOTF(std::string dataFileName, bool logging)
{
    #ifdef OTF1LIB
    std::cout << "Processing " << dataFileName.c_str() << std::endl;
    clock_t start = clock();


    OTFConverter * importer = new OTFConverter();
    Trace* trace = importer->importOTF(dataFileName, logging);
    delete importer;

    if (trace)
    {
        trace->preprocess();
    }

    clock_t end = clock();
    double traceElapsed = (end - start) / CLOCKS_PER_SEC;
    RavelUtils::gu_printTime(traceElapsed, "Total trace: ");

    return trace;
    #endif
}
