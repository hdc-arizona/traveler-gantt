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

void ImportFunctor::doImportOTF2(std::string dataFileName)
{
    std::cout << "Processing " << dataFileName.c_str() << std::endl;
    clock_t start = clock();

    OTFConverter * importer = new OTFConverter();
    connect(importer, SIGNAL(finishRead()), this, SLOT(finishInitialRead()));
    connect(importer, SIGNAL(matchingUpdate(int, std::string)), this,
            SLOT(updateMatching(int, std::string)));
    Trace* trace = importer->importOTF2(dataFileName);
    delete importer;

    if (trace)
    {
        connect(trace, SIGNAL(updatePreprocess(int, std::string)), this,
                SLOT(updatePreprocess(int, std::string)));
        trace->preprocess();
    }

    clock_t end = clock();
    double traceElapsed = (end - start) / CLOCKS_PER_SEC;
    RavelUtils::gu_printTime(traceElapsed, "Total trace: ");

    emit(done(trace));
}

void ImportFunctor::doImportOTF(std::string dataFileName)
{
    #ifdef OTF1LIB
    std::cout << "Processing " << dataFileName.c_str() << std::endl;
    clock_t start = clock();


    OTFConverter * importer = new OTFConverter();
    connect(importer, SIGNAL(finishRead()), this, SLOT(finishInitialRead()));
    connect(importer, SIGNAL(matchingUpdate(int, std::string)), this,
            SLOT(updateMatching(int, std::string)));
    Trace* trace = importer->importOTF(dataFileName);
    delete importer;

    if (trace)
    {
        connect(trace, SIGNAL(updatePreprocess(int, std::string)), this,
                SLOT(updatePreprocess(int, std::string)));
        trace->preprocess();
    }

    clock_t end = clock();
    double traceElapsed = (end - start) / CLOCKS_PER_SEC;
    RavelUtils::gu_printTime(traceElapsed, "Total trace: ");

    emit(done(trace));
    #endif
}

void ImportFunctor::finishInitialRead()
{
    emit(reportProgress(35, "Constructing events..."));
}

void ImportFunctor::updateMatching(int portion, std::string msg)
{
    emit(reportProgress(35 + portion, msg));
}

void ImportFunctor::updatePreprocess(int portion, std::string msg)
{
    emit(reportProgress(95 + portion / 2.0, msg));
}

void ImportFunctor::switchProgress()
{
    emit(switching());
}
