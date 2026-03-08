#ifndef BRUTEFORCESOLVER_H
#define BRUTEFORCESOLVER_H

#include <vector>
#include "TSPInstance.h"

struct BruteForceResult {
    std::vector<int> bestTour;
    int bestCost = -1;
    double timeMs = 0.0;
    long long checkedPermutations = 0;
};

class BruteForceSolver {
public:
    static BruteForceResult solve(const TSPInstance& instance, bool progress);
};

#endif