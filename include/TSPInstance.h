//
// Created by przemyslaw_dyjak on 8.03.2026.
//

#ifndef TSPINSTANCE_H
#define TSPINSTANCE_H

#include <string>
#include <vector>

struct TSPInstance {
    std::string name;
    std::string type;            // TSP albo ATSP
    std::string edgeWeightType;  // EXPLICIT, EUC_2D, ATT
    int dimension = 0;
    std::vector<std::vector<int>> matrix;
};

#endif