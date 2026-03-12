#include "../include/RNNSolver.h"
#include "TourUtils.h"
#include <chrono>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {
    // najwazniejsze, dostaje instancje, biezacy wiercholek, aktualnie budowana trase, globalnie najlepszy koszt i trase
    // startowy vertex i najlepszy start znaleziony do tej pory
    void dfsNearestTies(
        const TSPInstance& instance,
        int current,
        std::vector<bool>& visited,
        std::vector<int>& currentTour,
        int& globalBestCost,
        std::vector<int>& globalBestTour,
        int startVertex,
        int& globalBestStartVertex
    ) {
        int n = instance.dimension;

        // jak size n to kazde miasto w trasie, wiec koszt pelnej trasy, porownanie z global best i jak lepszy to zapisanie
        if (static_cast<int>(currentTour.size()) == n) {
            int cost = TourUtils::calculateTourCost(instance, currentTour);
            if (cost < globalBestCost) {
                globalBestCost = cost;
                globalBestTour = currentTour;
                globalBestStartVertex = startVertex;
            }
            return;
        }

        int minCost = std::numeric_limits<int>::max();
        std::vector<int> candidates;

        // dla kazdego nieodwiedzonego v jak edge cost < mincost i znalazl lepszy i wrzyca tylko ten koszt
        // ale jak edgecost jak mincost to dopisuje v do kandydatow - wszystkie lokalnie najlepsze ruchy
        for (int v = 0; v < n; v++) {
            if (visited[v]) {
                continue;
            }

            int edgeCost = instance.matrix[current][v];

            if (edgeCost < minCost) {
                minCost = edgeCost;
                candidates.clear();
                candidates.push_back(v);
            } else if (edgeCost == minCost) {
                candidates.push_back(v);
            }
        }

        if (candidates.empty()) {
            return;
        }

        // jak lista nie pusta to kanddat jak odiwedzony, dopisuje go do trasy i rekurencyjnie dfs, pozniej popback i nastepna trasa
        for (int next : candidates) {
            visited[next] = true;
            currentTour.push_back(next);

            dfsNearestTies(
                instance,
                next,
                visited,
                currentTour,
                globalBestCost,
                globalBestTour,
                startVertex,
                globalBestStartVertex
            );

            currentTour.pop_back();
            visited[next] = false;
        }
    }
}

RNNResult RNNSolver::solve(const TSPInstance& instance) {
    RNNResult result;

    int n = instance.dimension;
    if (n <= 0) {
        throw std::runtime_error("Instancja ma niepoprawny rozmiar");
    }

    auto startTime = std::chrono::steady_clock::now();
    // na start koszt z limits i start vertex tez hard coded
    int bestCost = std::numeric_limits<int>::max();
    std::vector<int> bestTour;
    int bestStartVertex = -1;

    for (int startVertex = 0; startVertex < n; startVertex++) {
        std::vector<bool> visited(n, false);
        std::vector<int> currentTour;

        visited[startVertex] = true;
        currentTour.push_back(startVertex);

        dfsNearestTies(
            instance,
            startVertex,
            visited,
            currentTour,
            bestCost,
            bestTour,
            startVertex,
            bestStartVertex
        );
    }

    auto endTime = std::chrono::steady_clock::now();

    result.bestTour = bestTour;
    result.bestCost = bestCost;
    result.bestStartVertex = bestStartVertex;
    result.timeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    return result;
}