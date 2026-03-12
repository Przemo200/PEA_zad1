#include "../include/MemoryUsage.h"
#include <fstream>
#include <sstream>
#include <string>


// pamiec fizycznie zajmoowana przez proces w RAM
long MemoryUsage::getCurrentRSSkB()  { // resisdent set size w kB, dziala tylko na linuksie
    std::ifstream file("/proc/self/status"); // status biezacego procesu
    std::string line;

    while (std::getline(file, line)) {
        //szuka VmRSS i wartosc stamtad pokazuje
        if (line.rfind("VmRSS:", 0) == 0) {
            std::stringstream ss(line);
            std::string label;
            long value = 0;
            std::string unit;
            ss >> label >> value >> unit;
            return value; // kB
        }
    }

    return -1;
}