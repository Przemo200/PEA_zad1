#include "../include/BruteForceSolver.h"
#include "../include/TourUtils.h"
#include <chrono>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

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

    void reverseRange(std::vector<int>& arr, int left, int right) {
        while (left < right) {
            int temp = arr[left];
            arr[left] = arr[right];
            arr[right] = temp;
            left++;
            right--;
        }
    }

    bool nextPermutationManual(std::vector<int>& arr) {
        int n = static_cast<int>(arr.size());
        if (n <= 1) {
            return false;
        }

        // 1. znajdź od końca pierwszy indeks i taki, że arr[i] < arr[i+1]
        int i = n - 2;
        while (i >= 0 && arr[i] >= arr[i + 1]) {
            i--;
        }

        if (i < 0) {
            return false; // to była ostatnia permutacja
        }

        // 2. znajdź od końca pierwszy element większy niż arr[i]
        int j = n - 1;
        while (arr[j] <= arr[i]) {
            j--;
        }

        // 3. zamień arr[i] z arr[j]
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;

        // 4. odwróć suffix
        reverseRange(arr, i + 1, n - 1);

        return true;
    }

    void buildInitialPermutation(std::vector<int>& perm, int n) {
        perm.resize(n - 1);
        for (int i = 0; i < n - 1; i++) {
            perm[i] = i + 1;
        }
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

    std::vector<int> perm;
    buildInitialPermutation(perm, n);

    bool symmetric = isSymmetricTSP(instance);

    int bestCost = std::numeric_limits<int>::max();
    std::vector<int> bestTour;
    long long checked = 0;

    auto startTime = std::chrono::steady_clock::now();

    bool hasMore = true;
    while (hasMore) {
        if (!(symmetric && !perm.empty() && perm.front() > perm.back())) {
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
        }

        hasMore = nextPermutationManual(perm);
    }

    auto endTime = std::chrono::steady_clock::now();

    result.bestTour = bestTour;
    result.bestCost = bestCost;
    result.checkedPermutations = checked;
    result.timeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    return result;
}