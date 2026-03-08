#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include "Config.h"
#include "FileReader.h"
#include "TourUtils.h"
#include "RandSolver.h"
#include "OptTourReader.h"
#include "NNSolver.h"
#include "RNNSolver.h"
#include "BruteForceSolver.h"
#include "Generator.h"
#include "CSVWriter.h"
#include "InstanceListReader.h"
#include "MemoryUsage.h"

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
    if (!config.opt_tour_file.empty()) {
        std::vector<int> optTour = OptTourReader::loadTour(config.opt_tour_file);

        if (!TourUtils::isValidTour(optTour, instance.dimension)) {
            throw std::runtime_error("Wczytana trasa optymalna jest niepoprawna.");
        }

        optCost = TourUtils::calculateTourCost(instance, optTour);
        return true;
    }

    if (config.single_opt_cost > 0) {
        optCost = config.single_opt_cost;
        return true;
    }

    return false;
}

static bool tryLoadOptCostFromEntry(const InstanceListEntry& entry, const TSPInstance& instance, int& optCost) {
    if (!entry.optTourFile.empty()) {
        std::vector<int> optTour = OptTourReader::loadTour(entry.optTourFile);

        if (!TourUtils::isValidTour(optTour, instance.dimension)) {
            throw std::runtime_error("Wczytana trasa optymalna z listy instancji jest niepoprawna.");
        }

        optCost = TourUtils::calculateTourCost(instance, optTour);
        return true;
    }

    if (entry.optCost > 0) {
        optCost = entry.optCost;
        return true;
    }

    return false;
}

static double computeRelativeErrorPercent(int cost, int optCost) {
    return 100.0 * (static_cast<double>(cost - optCost) / static_cast<double>(optCost));
}

static void printRelativeError(int cost, int optCost) {
    double error = computeRelativeErrorPercent(cost, optCost);
    std::cout << "Koszt optymalny: " << optCost << "\n";
    std::cout << "Blad wzgledny [%]: " << error << "\n";
}

static TSPInstance generateInstanceFromConfig(const Config& config, int n, unsigned int seed) {
    std::string typeUpper = config.generated_type;
    std::transform(typeUpper.begin(), typeUpper.end(), typeUpper.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });

    if (typeUpper == "ATSP") {
        return Generator::generateATSP(n, config.weight_min, config.weight_max, seed);
    } else if (typeUpper == "TSP") {
        return Generator::generateTSP(n, config.weight_min, config.weight_max, seed);
    }

    throw std::runtime_error("generated_type musi byc ATSP albo TSP.");
}

static void printMemoryUsage() {
    long rss = MemoryUsage::getCurrentRSSkB();
    if (rss >= 0) {
        std::cout << "Pamiec [kB]: " << rss << std::endl;
    } else {
        std::cout << "Pamiec [kB]: brak danych\n";
    }
}

int main(int argc, char* argv[]) {
    try {
        std::string configPath = "config/config.txt";
        if (argc > 1) {
            configPath = argv[1];
        }

        Config config = ConfigLoader::loadFromFile(configPath);

        if (config.mode == "benchmark_bf") {
            std::cout << "=== BENCHMARK BRUTE FORCE ===\n";
            std::cout << "Typ generowanych instancji: " << config.generated_type << "\n";
            std::cout << "Zakres n: " << config.bf_min_n << " - " << config.bf_max_n << "\n";
            std::cout << "Instancji na rozmiar: " << config.bf_instances_per_size << "\n";
            std::cout << "Plik CSV: " << config.output_csv << "\n\n";

            CSVWriter::writeHeaderIfNeeded(config.output_csv);

            for (int n = config.bf_min_n; n <= config.bf_max_n; n++) {
                std::cout << "Rozmiar n = " << n << "\n";

                for (int instanceId = 1; instanceId <= config.bf_instances_per_size; instanceId++) {
                    unsigned int localSeed = config.seed + static_cast<unsigned int>(n * 1000 + instanceId);

                    TSPInstance generated = generateInstanceFromConfig(config, n, localSeed);

                    BruteForceResult result = BruteForceSolver::solve(generated, false);

                    CSVWriter::appendBruteForceRow(
                        config.output_csv,
                        generated.type,
                        n,
                        instanceId,
                        result.bestCost,
                        result.timeMs,
                        result.checkedPermutations
                    );

                    std::cout << "  instancja " << instanceId
                              << " | koszt = " << result.bestCost
                              << " | czas [ms] = " << result.timeMs
                              << " | permutacje = " << result.checkedPermutations
                              << " | pamiec [kB] = " << MemoryUsage::getCurrentRSSkB()
                              << "\n";
                }

                std::cout << "\n";
            }

            std::cout << "Benchmark brute force zakonczony.\n";
            return 0;
        }

        if (config.mode == "benchmark_heuristics") {
            if (config.heuristics_list_file.empty()) {
                throw std::runtime_error("Brak heuristics_list_file w configu.");
            }

            std::vector<InstanceListEntry> entries = InstanceListReader::loadList(config.heuristics_list_file);

            std::cout << "=== BENCHMARK HEURYSTYK ===\n";
            std::cout << "Lista instancji: " << config.heuristics_list_file << "\n";
            std::cout << "Plik CSV: " << config.output_csv << "\n";
            std::cout << "RAND trials: " << config.rand_trials << "\n";
            std::cout << "RAND repeats: " << config.rand_repeats << "\n";
            std::cout << "NN start: " << config.nn_start_vertex << "\n\n";

            CSVWriter::writeHeuristicHeaderIfNeeded(config.output_csv);

            for (size_t i = 0; i < entries.size(); i++) {
                const InstanceListEntry& entry = entries[i];
                TSPInstance instance = FileReader::loadInstance(entry.instanceFile);

                int optCost = -1;
                bool hasOptCost = tryLoadOptCostFromEntry(entry, instance, optCost);

                std::cout << "Instancja: " << entry.name
                          << " | typ = " << instance.type
                          << " | n = " << instance.dimension << "\n";

                for (int rep = 1; rep <= config.rand_repeats; rep++) {
                    unsigned int localSeed = config.seed + static_cast<unsigned int>(i * 1000 + rep);

                    RandResult randResult = RandSolver::solve(
                        instance,
                        config.rand_trials,
                        localSeed,
                        false
                    );

                    double randError = hasOptCost ? computeRelativeErrorPercent(randResult.bestCost, optCost) : 0.0;

                    CSVWriter::appendHeuristicRow(
                        config.output_csv,
                        "RAND",
                        entry.name,
                        instance.type,
                        instance.dimension,
                        rep,
                        -1,
                        randResult.bestCost,
                        hasOptCost,
                        optCost,
                        randError,
                        randResult.timeMs
                    );

                    std::cout << "  RAND rep " << rep
                              << " | koszt = " << randResult.bestCost
                              << " | czas [ms] = " << randResult.timeMs
                              << " | pamiec [kB] = " << MemoryUsage::getCurrentRSSkB();
                    if (hasOptCost) {
                        std::cout << " | blad [%] = " << randError;
                    }
                    std::cout << "\n";
                }

                NNResult nnResult = NNSolver::solve(instance, config.nn_start_vertex);
                double nnError = hasOptCost ? computeRelativeErrorPercent(nnResult.cost, optCost) : 0.0;

                CSVWriter::appendHeuristicRow(
                    config.output_csv,
                    "NN",
                    entry.name,
                    instance.type,
                    instance.dimension,
                    1,
                    config.nn_start_vertex,
                    nnResult.cost,
                    hasOptCost,
                    optCost,
                    nnError,
                    nnResult.timeMs
                );

                std::cout << "  NN"
                          << " | koszt = " << nnResult.cost
                          << " | czas [ms] = " << nnResult.timeMs
                          << " | pamiec [kB] = " << MemoryUsage::getCurrentRSSkB();
                if (hasOptCost) {
                    std::cout << " | blad [%] = " << nnError;
                }
                std::cout << "\n";

                RNNResult rnnResult = RNNSolver::solve(instance);
                double rnnError = hasOptCost ? computeRelativeErrorPercent(rnnResult.bestCost, optCost) : 0.0;

                CSVWriter::appendHeuristicRow(
                    config.output_csv,
                    "RNN",
                    entry.name,
                    instance.type,
                    instance.dimension,
                    1,
                    rnnResult.bestStartVertex,
                    rnnResult.bestCost,
                    hasOptCost,
                    optCost,
                    rnnError,
                    rnnResult.timeMs
                );

                std::cout << "  RNN"
                          << " | koszt = " << rnnResult.bestCost
                          << " | czas [ms] = " << rnnResult.timeMs
                          << " | best start = " << rnnResult.bestStartVertex
                          << " | pamiec [kB] = " << MemoryUsage::getCurrentRSSkB();
                if (hasOptCost) {
                    std::cout << " | blad [%] = " << rnnError;
                }
                std::cout << "\n\n";
            }

            std::cout << "Benchmark heurystyk zakonczony.\n";
            return 0;
        }

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
            printMemoryUsage();

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
            printMemoryUsage();

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
            printMemoryUsage();

            int optCost = 0;
            if (tryLoadOptCost(config, instance, optCost)) {
                printRelativeError(result.bestCost, optCost);
            }
        }
        else if (config.mode == "bf") {
            std::cout << "\nUruchamiam brute force...\n";

            BruteForceResult result = BruteForceSolver::solve(instance, config.progress);

            std::cout << "\n=== WYNIK BRUTE FORCE ===\n";
            std::cout << "Najlepszy koszt: " << result.bestCost << "\n";
            std::cout << "Czas [ms]: " << result.timeMs << "\n";
            std::cout << "Sprawdzone permutacje: " << result.checkedPermutations << "\n";
            std::cout << "Najlepsza trasa:\n";
            std::cout << TourUtils::tourToString(result.bestTour) << "\n";
            printMemoryUsage();

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
            std::cout << "\nNieznany tryb.\n";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "BLAD: " << e.what() << "\n";
        return 1;
    }

    return 0;
}