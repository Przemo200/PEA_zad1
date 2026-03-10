#ifndef RANDSOLVER_H
#define RANDSOLVER_H

#include <vector>
#include "TSPInstance.h"

struct RandResult {
    std::vector<int> bestTour;
    int bestCost = -1;
    double timeMs = 0.0;
};

class RandSolver {
public:
    static RandResult solve(const TSPInstance& instance, int trials, unsigned int seed, bool progress);
};

#endif