//
// Created by przemyslaw_dyjak on 8.03.2026.
//

#ifndef TOURUTILS_H
#define TOURUTILS_H

#include <vector>
#include <string>
#include "TSPInstance.h"

namespace TourUtils {
    bool isValidTour(const std::vector<int>& tour, int n);
    int calculateTourCost(const TSPInstance& instance, const std::vector<int>& tour);
    std::string tourToString(const std::vector<int>& tour);
}

#endif