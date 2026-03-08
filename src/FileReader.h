//
// Created by przemyslaw_dyjak on 8.03.2026.
//

#ifndef FILEREADER_H
#define FILEREADER_H

#include <string>
#include "TSPInstance.h"

class FileReader {
public:
    static TSPInstance loadInstance(const std::string& path);
};

#endif