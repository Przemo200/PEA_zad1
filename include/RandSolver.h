#ifndef RANDSOLVER_H
#define RANDSOLVER_H

#include <vector>
#include "TSPInstance.h"

struct RandResult {
    std::vector<int> bestTour;
    int bestCost = -1;
    double timeMs = 0.0;

    // opcjonalnie do wczesniejszego zatrzymania RAND po osiagnieciu bledu wzg. np. <= 10%, np jakbym chcial macierz 6x6 a 10mln sciezek rand
    // jeszcze do odkomentowania by bylo w rand.cpp i main.cpp
    // int trialsDone = 0;
    // bool stoppedEarly = false;
};

class RandSolver {
public:
    static RandResult solve(const TSPInstance& instance, int trials, unsigned int seed, bool progress);

    // jak wyzej - rozszerzona wersja solve z early stop po bledzie wzglednym do podmianki mogloby byc - jeszcze do odkomentowania w rand.cpp i main.cpp
    // static RandResult solve(const TSPInstance& instance,
    //                         int trials,
    //                         unsigned int seed,
    //                         bool progress,
    //                         bool earlyStopEnabled,
    //                         int optCost,
    //                         double maxRelativeErrorPercent);
};

#endif