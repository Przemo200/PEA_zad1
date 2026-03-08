#include "BruteForceSolver.h"
#include "TourUtils.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
#include <numeric>
#include <stdexcept>

namespace {
    bool isSymmetricTSP(const TSPInstance& instance) {
        if (instance.type != "TSP") {
            return false;
        }

        int n = instance.dimension;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (instance.matrix[i][j] != instance.matrix[j][i]) {
                    return false;
                }
            }
        }

        return true;
    }
}

BruteForceResult BruteForceSolver::solve(const TSPInstance& instance, bool progress) {
    BruteForceResult result;

    int n = instance.dimension;
    if (n <= 0) {
        throw std::runtime_error("Instancja ma niepoprawny rozmiar.");
    }

    if (n == 1) {
        result.bestTour = {0};
        result.bestCost = 0;
        result.timeMs = 0.0;
        result.checkedPermutations = 1;
        return result;
    }

    std::vector<int> perm(n - 1);
    std::iota(perm.begin(), perm.end(), 1);

    bool symmetric = isSymmetricTSP(instance);

    int bestCost = std::numeric_limits<int>::max();
    std::vector<int> bestTour;
    long long checked = 0;

    auto startTime = std::chrono::steady_clock::now();

    do {
        if (symmetric) {
            if (!perm.empty() && perm.front() > perm.back()) {
                continue;
            }
        }

        std::vector<int> tour;
        tour.reserve(n);
        tour.push_back(0);
        for (int v : perm) {
            tour.push_back(v);
        }

        int cost = TourUtils::calculateTourCost(instance, tour);
        checked++;

        if (cost < bestCost) {
            bestCost = cost;
            bestTour = tour;

            if (progress) {
                std::cout << "[BF] Nowy najlepszy koszt: " << bestCost
                          << " po sprawdzeniu " << checked << " permutacji\n";
            }
        }

    } while (std::next_permutation(perm.begin(), perm.end()));

    auto endTime = std::chrono::steady_clock::now();

    result.bestTour = bestTour;
    result.bestCost = bestCost;
    result.checkedPermutations = checked;
    result.timeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    return result;
}