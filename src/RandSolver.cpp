#include "../include/RandSolver.h"
#include "TourUtils.h"
#include <chrono>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

namespace {
    void generateIdentityTour(std::vector<int>& tour, int n) {
        tour.resize(n);
        for (int i = 0; i < n; i++) {
            tour[i] = i;
        }
    }

    void fisherYatesShuffleFromSecond(std::vector<int>& tour, std::mt19937& rng) {
        int n = static_cast<int>(tour.size());

        // zostawiamy tour[0] = 0 jako ustalony start
        for (int i = n - 1; i >= 2; i--) {
            std::uniform_int_distribution<int> dist(1, i);
            int j = dist(rng);

            int temp = tour[i];
            tour[i] = tour[j];
            tour[j] = temp;
        }
    }
}

RandResult RandSolver::solve(const TSPInstance& instance, int trials, unsigned int seed, bool progress) {
    RandResult result;

    int n = instance.dimension;
    if (n <= 0) {
        result.bestCost = -1;
        result.timeMs = 0.0;
        return result;
    }

    if (n == 1) {
        result.bestTour = {0};
        result.bestCost = 0;
        result.timeMs = 0.0;
        return result;
    }

    std::mt19937 rng(seed);

    std::vector<int> currentTour;
    generateIdentityTour(currentTour, n);

    int bestCost = std::numeric_limits<int>::max();
    std::vector<int> bestTour = currentTour;

    auto start = std::chrono::steady_clock::now();

    for (int t = 0; t < trials; t++) {
        generateIdentityTour(currentTour, n);
        fisherYatesShuffleFromSecond(currentTour, rng);

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