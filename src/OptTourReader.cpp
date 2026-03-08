#include "OptTourReader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <cctype>

namespace {
    std::string trim(const std::string& s) {
        size_t start = 0;
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
            start++;
        }

        size_t end = s.size();
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
            end--;
        }

        return s.substr(start, end - start);
    }
}

std::vector<int> OptTourReader::loadTour(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Nie mozna otworzyc pliku optymalnej trasy: " + path);
    }

    std::vector<int> tour;
    std::string line;
    bool inTourSection = false;

    while (std::getline(file, line)) {
        line = trim(line);

        if (line.empty()) {
            continue;
        }

        if (!inTourSection) {
            if (line.find("TOUR_SECTION") != std::string::npos) {
                inTourSection = true;
            }
            continue;
        }

        if (line == "-1" || line == "EOF") {
            break;
        }

        int city = std::stoi(line);
        tour.push_back(city - 1);
    }

    return tour;
}