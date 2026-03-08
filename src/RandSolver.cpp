#include "RandSolver.h"
#include "TourUtils.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>

RandResult RandSolver::solve(const TSPInstance& instance, int trials, unsigned int seed, bool progress) {
    RandResult result;

    int n = instance.dimension;
    if (n <= 1) {
        result.bestCost = 0;
        result.bestTour = {0};
        result.timeMs = 0.0;
        return result;
    }

    std::vector<int> currentTour(n);
    std::iota(currentTour.begin(), currentTour.end(), 0);

    std::mt19937 rng(seed);

    int bestCost = std::numeric_limits<int>::max();
    std::vector<int> bestTour = currentTour;

    auto start = std::chrono::steady_clock::now();

    for (int t = 0; t < trials; t++) {
        std::shuffle(currentTour.begin() + 1, currentTour.end(), rng);

        int cost = TourUtils::calculateTourCost(instance, currentTour);

        if (cost < bestCost) {
            bestCost = cost;
            bestTour = currentTour;
        }

        if (progress && trials >= 10) {
            int step = trials / 10;
            if (step == 0) {
                step = 1;
            }

            if ((t + 1) % step == 0 || t + 1 == trials) {
                int percent = static_cast<int>((100.0 * (t + 1)) / trials);
                std::cout << "[RAND] Postep: " << percent << "%\n";
            }
        }
    }

    auto end = std::chrono::steady_clock::now();

    result.bestTour = bestTour;
    result.bestCost = bestCost;
    result.timeMs = std::chrono::duration<double, std::milli>(end - start).count();

    return result;
}