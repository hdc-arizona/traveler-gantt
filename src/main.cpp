/* Ravel */
#include <string>
#include <vector>
#include "trace.h";
#include "importfunctor.h"
#include <iostream>

int main(int argc, char **argv)
{
    if (argc < 1)
        return -1;

    // Now get the Trace File
    std::string dataFileName = argv[1];

    if (dataFileName.length() == 0)
        return -1;

    ImportFunctor * importWorker = new ImportFunctor();

    Trace * trace = NULL;
    if (dataFileName.compare(dataFileName.length() - 3, 3, "otf"))
    {
        trace = importWorker->doImportOTF(dataFileName);
    }
    else if (dataFileName.compare(dataFileName.length() - 3, 3, "otf2"))
    {
        trace = importWorker->doImportOTF2(dataFileName);
    }
    else
    {
        std::cout << "Unrecognized trace format!" << std::endl;
    }
}
