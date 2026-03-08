#include "MemoryUsage.h"
#include <fstream>
#include <sstream>
#include <string>

long MemoryUsage::getCurrentRSSkB() {
    std::ifstream file("/proc/self/status");
    std::string line;

    while (std::getline(file, line)) {
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