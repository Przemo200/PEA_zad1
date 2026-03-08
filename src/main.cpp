#include <iostream>
#include <iomanip>
#include <stdexcept>
#include "Config.h"
#include "FileReader.h"
#include "TourUtils.h"
#include "RandSolver.h"
#include "OptTourReader.h"
#include "NNSolver.h"
#include "RNNSolver.h"

static void printMatrix(const TSPInstance& instance) {
    std::cout << "\nMacierz kosztow:\n";
    for (int i = 0; i < instance.dimension; i++) {
        for (int j = 0; j < instance.dimension; j++) {
            std::cout << std::setw(6) << instance.matrix[i][j] << " ";
        }
        std::cout << '\n';
    }
}

static bool tryLoadOptCost(const Config& config, const TSPInstance& instance, int& optCost) {
    if (config.opt_tour_file.empty()) {
        return false;
    }

    std::vector<int> optTour = OptTourReader::loadTour(config.opt_tour_file);

    if (!TourUtils::isValidTour(optTour, instance.dimension)) {
        throw std::runtime_error("Wczytana trasa optymalna jest niepoprawna.");
    }

    optCost = TourUtils::calculateTourCost(instance, optTour);
    return true;
}

static void printRelativeError(int cost, int optCost) {
    double error = 100.0 * (static_cast<double>(cost - optCost) / static_cast<double>(optCost));
    std::cout << "Koszt optymalny: " << optCost << "\n";
    std::cout << "Blad wzgledny [%]: " << error << "\n";
}

int main(int argc, char* argv[]) {
    try {
        std::string configPath = "config/config.txt";
        if (argc > 1) {
            configPath = argv[1];
        }

        Config config = ConfigLoader::loadFromFile(configPath);
        TSPInstance instance = FileReader::loadInstance(config.instance_file);

        std::cout << "=== PEA zadanie 1 ===\n";
        std::cout << "Tryb: " << config.mode << "\n";
        std::cout << "Plik instancji: " << config.instance_file << "\n";
        std::cout << "Nazwa: " << instance.name << "\n";
        std::cout << "Typ: " << instance.type << "\n";
        std::cout << "EDGE_WEIGHT_TYPE: " << instance.edgeWeightType << "\n";
        std::cout << "Rozmiar: " << instance.dimension << "\n";

        if (config.show_matrix) {
            printMatrix(instance);
        }

        if (config.mode == "test_read") {
            std::cout << "\nEtap 1 nadal dziala poprawnie.\n";
        }
        else if (config.mode == "rand") {
            std::cout << "\nUruchamiam RAND...\n";
            std::cout << "Liczba losowan: " << config.rand_trials << "\n";
            std::cout << "Seed: " << config.seed << "\n";

            RandResult result = RandSolver::solve(
                instance,
                config.rand_trials,
                config.seed,
                config.progress
            );

            std::cout << "\n=== WYNIK RAND ===\n";
            std::cout << "Najlepszy koszt: " << result.bestCost << "\n";
            std::cout << "Czas [ms]: " << result.timeMs << "\n";
            std::cout << "Najlepsza trasa:\n";
            std::cout << TourUtils::tourToString(result.bestTour) << "\n";

            int optCost = 0;
            if (tryLoadOptCost(config, instance, optCost)) {
                printRelativeError(result.bestCost, optCost);
            }
        }
        else if (config.mode == "nn") {
            std::cout << "\nUruchamiam NN...\n";
            std::cout << "Wierzcholek startowy: " << config.nn_start_vertex << "\n";

            NNResult result = NNSolver::solve(instance, config.nn_start_vertex);

            std::cout << "\n=== WYNIK NN ===\n";
            std::cout << "Koszt: " << result.cost << "\n";
            std::cout << "Czas [ms]: " << result.timeMs << "\n";
            std::cout << "Trasa:\n";
            std::cout << TourUtils::tourToString(result.tour) << "\n";

            int optCost = 0;
            if (tryLoadOptCost(config, instance, optCost)) {
                printRelativeError(result.cost, optCost);
            }
        }
        else if (config.mode == "rnn") {
            std::cout << "\nUruchamiam RNN...\n";

            RNNResult result = RNNSolver::solve(instance);

            std::cout << "\n=== WYNIK RNN ===\n";
            std::cout << "Najlepszy koszt: " << result.bestCost << "\n";
            std::cout << "Najlepszy start: " << result.bestStartVertex << "\n";
            std::cout << "Czas [ms]: " << result.timeMs << "\n";
            std::cout << "Najlepsza trasa:\n";
            std::cout << TourUtils::tourToString(result.bestTour) << "\n";

            int optCost = 0;
            if (tryLoadOptCost(config, instance, optCost)) {
                printRelativeError(result.bestCost, optCost);
            }
        }
        else if (config.mode == "check_opt") {
            if (config.opt_tour_file.empty()) {
                throw std::runtime_error("Brak opt_tour_file w configu.");
            }

            std::vector<int> optTour = OptTourReader::loadTour(config.opt_tour_file);

            std::cout << "\nSprawdzam trase optymalna...\n";
            std::cout << "Plik .opt.tour: " << config.opt_tour_file << "\n";

            if (!TourUtils::isValidTour(optTour, instance.dimension)) {
                throw std::runtime_error("Wczytana trasa optymalna jest niepoprawna.");
            }

            int optCost = TourUtils::calculateTourCost(instance, optTour);

            std::cout << "\n=== WYNIK CHECK_OPT ===\n";
            std::cout << "Koszt trasy z pliku .opt.tour: " << optCost << "\n";
            std::cout << "Trasa:\n";
            std::cout << TourUtils::tourToString(optTour) << "\n";
        }
        else {
            std::cout << "\nNieznany tryb. Ustaw mode=test_read, mode=rand, mode=nn, mode=rnn albo mode=check_opt.\n";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "BLAD: " << e.what() << "\n";
        return 1;
    }

    return 0;
}