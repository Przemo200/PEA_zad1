#ifndef NNSOLVER_H
#define NNSOLVER_H

#include <vector>
#include "TSPInstance.h"

struct NNResult {
    std::vector<int> tour;
    int cost = -1;
    double timeMs = 0.0;
    int startVertex = 0;
};

class NNSolver {
public:
    static NNResult solve(const TSPInstance& instance, int startVertex);
};

#endif