#include "../include/RandSolver.h"
#include "TourUtils.h"
#include <chrono>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

namespace {
    // lista miast ktora pozniej sie tasuje
    void generateIdentityTour(std::vector<int>& tour, int n) {
        tour.resize(n);
        for (int i = 0; i < n; i++) {
            tour[i] = i;
        }
    }
    // nie od indeks 0 tylko od 1
    // idzie od konca tablicy do poczatku, dla kazdego i losuje j z zakresu 1 do i i zamienia [i] z [j]
    // zlozonosc fisher yatesa to O(n). jest tez algorytm Heapa, ale nie zaglebialem sie w niego (zamiana pary)
    void fisherYatesShuffleFromSecond(std::vector<int>& tour, std::mt19937& rng) {
        int n = static_cast<int>(tour.size());

        // zostawiamy tour[0] = 0 jako ustalony start, bo 0-1-2-0 to to samo co 1-2-0-1
        for (int i = n - 1; i >= 2; i--) {
            std::uniform_int_distribution<int> dist(1, i); // rownomierne prawdopodobienstwo na kazda z przedzialu
            int j = dist(rng);

            int temp = tour[i];
            tour[i] = tour[j];
            tour[j] = temp;
        }
    }
}


// opcjonalnie - helper do early stop po bledzie wzglednym - trzeba tez odkomentowac rzeczy w rand.h i main.cpp
// static double computeRelativeErrorPercentForRand(int cost, int optCost) {
//     return 100.0 * (static_cast<double>(cost - optCost) / static_cast<double>(optCost));
// }



// przyjmuje instancje, okreslona liczbe sciezek, seed i info czy pokazac progres
// jako ze FY to O(n) a koszt trasy to tez O(n) to ogolna zlozonosc tego bedzie O(trials * n)
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
    // na start jakis wielki wynik
    int bestCost = std::numeric_limits<int>::max();
    std::vector<int> bestTour = currentTour;

    auto start = std::chrono::steady_clock::now();

    for (int t = 0; t < trials; t++) {
        // trasa bazowa, fisher yates, koszt i jak lepszy od aktualnego best to nadpisywany
        generateIdentityTour(currentTour, n);
        fisherYatesShuffleFromSecond(currentTour, rng);

        int cost = TourUtils::calculateTourCost(instance, currentTour);

        if (cost < bestCost) {
            bestCost = cost;
            bestTour = currentTour;
        }

        // opcjonalnie early stop dla RAND po osiagnieciu bledu wzglednego <= prog mogloby dzialac tak (jeszcze rzeczy w .h i main.cpp tez)
        // tylko wtedy, gdy znamy optCost

        // bool earlyStopEnabled = true;
        // int optCost = ...;
        // double maxRelativeErrorPercent = 10.0;
        //
        // if (earlyStopEnabled && optCost > 0) {
        //     double relError = computeRelativeErrorPercentForRand(bestCost, optCost);
        //     if (relError <= maxRelativeErrorPercent) {
        //         // jezeli doda sie do RandResult pola trialsDone i stoppedEarly too tutaj mozna by je uzupelnic
        //         // result.trialsDone = t + 1;
        //         // result.stoppedEarly = true;
        //         break;
        //     }
        // }

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

    auto end = std::chrono::steady_clock::now();  // nie skacze, niezalezny na zmiany

    result.bestTour = bestTour;
    result.bestCost = bestCost;
    result.timeMs = std::chrono::duration<double, std::milli>(end - start).count();

    return result;
}