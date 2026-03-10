#include "../include/NNSolver.h"
#include "../include/TourUtils.h"
#include <chrono>
#include <limits>
#include <stdexcept>

NNResult NNSolver::solve(const TSPInstance& instance, int startVertex) {
    NNResult result;
    result.startVertex = startVertex;

    int n = instance.dimension;
    if (n <= 0) {
        throw std::runtime_error("Instancja ma niepoprawny rozmiar.");
    }

    if (startVertex < 0 || startVertex >= n) {
        throw std::runtime_error("Niepoprawny wierzcholek startowy dla NN.");
    }

    auto startTime = std::chrono::steady_clock::now();

    std::vector<bool> visited(n, false);
    std::vector<int> tour;
    tour.reserve(n);

    int current = startVertex;
    visited[current] = true;
    tour.push_back(current);

    for (int step = 1; step < n; step++) {
        int bestNext = -1;
        int bestCost = std::numeric_limits<int>::max();

        for (int v = 0; v < n; v++) {
            if (visited[v]) {
                continue;
            }

            int edgeCost = instance.matrix[current][v];
            if (edgeCost < bestCost) {
                bestCost = edgeCost;
                bestNext = v;
            }
        }

        if (bestNext == -1) {
            throw std::runtime_error("NN nie znalazl kolejnego wierzcholka.");
        }

        visited[bestNext] = true;
        tour.push_back(bestNext);
        current = bestNext;
    }

    auto endTime = std::chrono::steady_clock::now();

    result.tour = tour;
    result.cost = TourUtils::calculateTourCost(instance, tour);
    result.timeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    return result;
}