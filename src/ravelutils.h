#ifndef RAVEL_UTIL_H
#define RAVEL_UTIL_H

#include <string>
#include <iostream>

// For qSorting lists of pointers
template<class T>
bool dereferencedLessThan(T * o1, T * o2) {
    return *o1 < *o2;
}

class RavelUtils
{
public:
    // For units
    static std::string getUnits(int zeros)
    {
        if (zeros >= 18)
            return "as";
        else if (zeros >= 15)
            return "fs";
        else if (zeros >= 12)
            return "ps";
        else if (zeros >= 9)
            return "ns";
        else if (zeros >= 6)
            return "us";
        else if (zeros >= 3)
            return "ms";
        else
            return "s";
    }

    // For timing information
    static void gu_printTime(double seconds, std::string label)
    {
        std::cout << label.c_str();
        if (seconds < 300)
        {
            std::cout << seconds << " seconds " << std::endl;
            return;
        }

        double minutes = seconds / 60.0;
        if (minutes < 300)
        {
            std::cout << minutes << " minutes " << std::endl;
            return;
        }

        double hours = minutes / 60.0;
        std::cout << hours << " hours" << std::endl;
        return;
    }
};

#endif // RAVEL_UTIL_H
