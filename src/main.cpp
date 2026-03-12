#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <map>
#include <string>

#include "../include/Config.h"
#include "../include/FileReader.h"
#include "../include/TourUtils.h"
#include "../include/RandSolver.h"
#include "../include/OptTourReader.h"
#include "../include/NNSolver.h"
#include "../include/RNNSolver.h"
#include "../include/BruteForceSolver.h"
#include "../include/Generator.h"
#include "../include/CSVWriter.h"
#include "../include/InstanceListReader.h"
#include "../include/MemoryUsage.h"

// struktura do zbierania sum czasow i bledow algorytmow zeby wyciagnac srednie
struct HeuristicSummary {
    double randTimeSum = 0.0;
    double randErrorSum = 0.0;
    int randRuns = 0; //liczba uruhcomien uwzglednionych do liczenia sredniego czasu
    int randErrorRuns = 0; // liczba uruchomien dla ktrych mozna bylo policzyc blad wzgledny

    double nnTimeSum = 0.0;
    double nnErrorSum = 0.0;
    int nnRuns = 0;
    int nnErrorRuns = 0;

    double rnnTimeSum = 0.0;
    double rnnErrorSum = 0.0;
    int rnnRuns = 0;
    int rnnErrorRuns = 0;
};

// info o optimum - jakie i czy "z palca" czy z pliku
struct OptCostInfo {
    bool available = false;
    int cost = -1;
    std::string source;
};

// tylko gdy show matrix jest true w configach
static void printMatrix(const TSPInstance& instance) {
    std::cout << "\nMacierz kosztow:\n";
    for (int i = 0; i < instance.dimension; i++) {
        for (int j = 0; j < instance.dimension; j++) {
            std::cout << std::setw(6) << instance.matrix[i][j] << " ";
        }
        std::cout << '\n';
    }
}

// to jest dla pojedynczych uruchomien
static OptCostInfo getOptCostFromConfig(const Config& config, const TSPInstance& instance) {
    OptCostInfo info;

    //jak plik z opt podany to przez reader
    if (!config.opt_tour_file.empty()) {
        std::vector<int> optTour = OptTourReader::loadTour(config.opt_tour_file);
        //sprawdza poprawnosc trasy optymalnej
        if (!TourUtils::isValidTour(optTour, instance.dimension)) { // czy n wierzcholkow, czy sie nie powtarzaja
            throw std::runtime_error("Wczytana trasa optymalna jest niepoprawna");
        }

        info.available = true;
        info.cost = TourUtils::calculateTourCost(instance, optTour);
        info.source = ".opt.tour";
        return info;
    }
    // jak nie plik to to co z palca w liscie instancji
    if (config.single_opt_cost > 0) {
        info.available = true;
        info.cost = config.single_opt_cost;
        info.source = "single_opt_cost";
        return info;
    }

    return info;
}

// tu bierze z pliku z listy instacji, entry jest dla benchmarkow
static OptCostInfo getOptCostFromEntry(const InstanceListEntry& entry, const TSPInstance& instance) {
    OptCostInfo info;

    if (!entry.optTourFile.empty()) {
        std::vector<int> optTour = OptTourReader::loadTour(entry.optTourFile);

        if (!TourUtils::isValidTour(optTour, instance.dimension)) {
            throw std::runtime_error("Wczytana trasa optymalna z listy instancji jest niepoprawna");
        }

        info.available = true;
        info.cost = TourUtils::calculateTourCost(instance, optTour);
        info.source = ".opt.tour";
        return info;
    }

    if (entry.optCost > 0) {
        info.available = true;
        info.cost = entry.optCost;
        info.source = "lista instancji";
        return info;
    }

    return info;
}

// blad wzgledny zgodnie ze wzorem
static double computeRelativeErrorPercent(int cost, int optCost) {
    return 100.0 * (static_cast<double>(cost - optCost) / static_cast<double>(optCost));
}

// optymalny i blad wzgledny - wypisywanie
static void printRelativeError(int cost, int optCost) {
    double error = computeRelativeErrorPercent(cost, optCost);
    std::cout << "Koszt optymalny / best known: " << optCost << "\n";
    std::cout << "Blad wzgledny [%]: " << error << "\n";
}

// optymalny i skad pochodzi lub info o braku
static void printOptInfo(const OptCostInfo& info) {
    if (info.available) {
        std::cout << "Koszt optymalny / best known: " << info.cost
                  << " (zrodlo: " << info.source << ")\n";
    } else {
        std::cout << "Koszt optymalny / best known: brak danych "
                     "(dodaj opt_tour_file albo single_opt_cost)\n";
    }
}

// patrzy na typ podany w configu dla brute forca i generuje albo tsp albo atsp
static TSPInstance generateInstanceFromConfig(const Config& config, int n, unsigned int seed) {
    std::string typeUpper = config.generated_type;
    std::transform(typeUpper.begin(), typeUpper.end(), typeUpper.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });

    if (typeUpper == "ATSP") {
        return Generator::generateATSP(n, config.weight_min, config.weight_max, seed);
    } else if (typeUpper == "TSP") {
        return Generator::generateTSP(n, config.weight_min, config.weight_max, seed);
    }

    throw std::runtime_error("generated_type musi byc ATSP albo TSP");
}

// nakladka dla memory usage, albo uda sie odczytac albo -1
static long getMemoryUsageOrMinusOne() {
    return MemoryUsage::getCurrentRSSkB();
}

// wypisanie zuzycia pamieci VmRSS
static void printMemoryUsage() {
    long rss = getMemoryUsageOrMinusOne();
    if (rss >= 0) {
        std::cout << "Pamiec [kB]: " << rss << "\n";
    } else {
        std::cout << "Pamiec [kB]: brak danych\n";
    }
}

// dla sredniego czasu, pamieci
static double safeAverage(double sum, int count) {
    return (count > 0) ? (sum / static_cast<double>(count)) : 0.0;
}

static void printBenchmarkProgress(bool enabled,
                                   int currentStep,
                                   int totalSteps,
                                   const std::string& label) {
    if (!enabled || totalSteps <= 0) {
        return;
    }

    double percent = 100.0 * static_cast<double>(currentStep) / static_cast<double>(totalSteps);
    std::cout << "[POSTEP] " << currentStep << "/" << totalSteps
              << " (" << percent << "%) - " << label << "\n";
}

static int countHeuristicBenchmarkSteps(const Config& config,
                                        const std::vector<InstanceListEntry>& entries) {
    return static_cast<int>(entries.size()) * (config.rand_repeats + 2);
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
            std::cout << "Plik CSV: " << config.output_csv << "\n";
            std::cout << "Progress: " << (config.progress ? "on" : "off") << "\n\n";

            CSVWriter::writeHeaderIfNeeded(config.output_csv);

            for (int n = config.bf_min_n; n <= config.bf_max_n; n++) {
                std::cout << "Rozmiar n = " << n << "\n";

                double sumTimeMs = 0.0;
                long long sumPermutations = 0;
                long long sumMemoryKb = 0;
                int memorySamples = 0;

                for (int instanceId = 1; instanceId <= config.bf_instances_per_size; instanceId++) {
                    unsigned int localSeed = config.seed + static_cast<unsigned int>(n * 1000 + instanceId);

                    TSPInstance generated = generateInstanceFromConfig(config, n, localSeed);
                    BruteForceResult result = BruteForceSolver::solve(generated, config.progress);
                    long rss = getMemoryUsageOrMinusOne();

                    CSVWriter::appendBruteForceRow(
                        config.output_csv,
                        generated.type,
                        n,
                        instanceId,
                        result.bestCost,
                        result.timeMs,
                        result.checkedPermutations
                    );

                    sumTimeMs += result.timeMs;
                    sumPermutations += result.checkedPermutations;
                    if (rss >= 0) {
                        sumMemoryKb += rss;
                        memorySamples++;
                    }

                    std::cout << "  instancja " << instanceId
                              << " | koszt = " << result.bestCost
                              << " | czas [ms] = " << result.timeMs
                              << " | permutacje = " << result.checkedPermutations;

                    if (rss >= 0) {
                        std::cout << " | pamiec [kB] = " << rss;
                    } else {
                        std::cout << " | pamiec [kB] = brak danych";
                    }
                    std::cout << "\n";
                }

                std::cout << "  SREDNIO dla n = " << n
                          << " | czas [ms] = " << safeAverage(sumTimeMs, config.bf_instances_per_size)
                          << " | permutacje = " << safeAverage(static_cast<double>(sumPermutations), config.bf_instances_per_size);

                if (memorySamples > 0) {
                    std::cout << " | pamiec [kB] = " << safeAverage(static_cast<double>(sumMemoryKb), memorySamples);
                } else {
                    std::cout << " | pamiec [kB] = brak danych";
                }
                std::cout << "\n\n";
            }

            std::cout << "Benchmark brute force zakonczony.\n";
            return 0;
        }

        if (config.mode == "benchmark_heuristics") {
            std::vector<InstanceListEntry> entries = InstanceListReader::loadList(config.heuristics_list_file);
            std::map<int, HeuristicSummary> summariesByDimension;

            int totalSteps = countHeuristicBenchmarkSteps(config, entries);
            int currentStep = 0;

            std::cout << "=== BENCHMARK HEURYSTYK ===\n";
            std::cout << "Lista instancji: " << config.heuristics_list_file << "\n";
            std::cout << "Plik CSV: " << config.output_csv << "\n";
            std::cout << "RAND trials: " << config.rand_trials << "\n";
            std::cout << "RAND repeats: " << config.rand_repeats << "\n";
            std::cout << "NN start: " << config.nn_start_vertex << "\n";
            std::cout << "Progress: " << (config.progress ? "on" : "off") << "\n\n";

            CSVWriter::writeHeuristicHeaderIfNeeded(config.output_csv);

            for (size_t i = 0; i < entries.size(); i++) {
                const InstanceListEntry& entry = entries[i];
                TSPInstance instance = FileReader::loadInstance(entry.instanceFile);
                HeuristicSummary& summary = summariesByDimension[instance.dimension];

                OptCostInfo optInfo = getOptCostFromEntry(entry, instance);

                std::cout << "Instancja: " << entry.name
                          << " | typ = " << instance.type
                          << " | n = " << instance.dimension << "\n";
                printOptInfo(optInfo);

                double randTimeForInstance = 0.0;
                double randErrorForInstance = 0.0;
                int randErrorRunsForInstance = 0;

                for (int rep = 1; rep <= config.rand_repeats; rep++) {
                    currentStep++;
                    printBenchmarkProgress(
                        config.progress,
                        currentStep,
                        totalSteps,
                        "RAND | instancja=" + entry.name + " | rep=" + std::to_string(rep)
                    );

                    unsigned int localSeed = config.seed + static_cast<unsigned int>(i * 1000 + rep);

                    RandResult randResult = RandSolver::solve(
                        instance,
                        config.rand_trials,
                        localSeed,
                        false
                    );

                    double randError = optInfo.available ? computeRelativeErrorPercent(randResult.bestCost, optInfo.cost) : 0.0;

                    CSVWriter::appendHeuristicRow(
                        config.output_csv,
                        "RAND",
                        entry.name,
                        instance.type,
                        instance.dimension,
                        rep,
                        -1,
                        randResult.bestCost,
                        optInfo.available,
                        optInfo.cost,
                        randError,
                        randResult.timeMs
                    );

                    long rss = getMemoryUsageOrMinusOne();
                    std::cout << "  RAND rep " << rep
                              << " | koszt = " << randResult.bestCost
                              << " | czas [ms] = " << randResult.timeMs;
                    if (rss >= 0) {
                        std::cout << " | pamiec [kB] = " << rss;
                    } else {
                        std::cout << " | pamiec [kB] = brak danych";
                    }
                    if (optInfo.available) {
                        std::cout << " | opt = " << optInfo.cost
                                  << " | blad [%] = " << randError;
                    }
                    std::cout << "\n";

                    randTimeForInstance += randResult.timeMs;
                    summary.randTimeSum += randResult.timeMs;
                    summary.randRuns++;

                    if (optInfo.available) {
                        randErrorForInstance += randError;
                        randErrorRunsForInstance++;
                        summary.randErrorSum += randError;
                        summary.randErrorRuns++;
                    }
                }

                std::cout << "  RAND srednio dla instancji"
                          << " | czas [ms] = " << safeAverage(randTimeForInstance, config.rand_repeats);
                if (randErrorRunsForInstance > 0) {
                    std::cout << " | avg blad [%] = " << safeAverage(randErrorForInstance, randErrorRunsForInstance);
                }
                std::cout << "\n";

                currentStep++;
                printBenchmarkProgress(
                    config.progress,
                    currentStep,
                    totalSteps,
                    "NN | instancja=" + entry.name
                );

                NNResult nnResult = NNSolver::solve(instance, config.nn_start_vertex);
                double nnError = optInfo.available ? computeRelativeErrorPercent(nnResult.cost, optInfo.cost) : 0.0;

                CSVWriter::appendHeuristicRow(
                    config.output_csv,
                    "NN",
                    entry.name,
                    instance.type,
                    instance.dimension,
                    1,
                    config.nn_start_vertex,
                    nnResult.cost,
                    optInfo.available,
                    optInfo.cost,
                    nnError,
                    nnResult.timeMs
                );

                long nnRss = getMemoryUsageOrMinusOne();
                std::cout << "  NN"
                          << " | koszt = " << nnResult.cost
                          << " | czas [ms] = " << nnResult.timeMs;
                if (nnRss >= 0) {
                    std::cout << " | pamiec [kB] = " << nnRss;
                } else {
                    std::cout << " | pamiec [kB] = brak danych";
                }
                if (optInfo.available) {
                    std::cout << " | opt = " << optInfo.cost
                              << " | blad [%] = " << nnError;
                }
                std::cout << "\n";

                summary.nnTimeSum += nnResult.timeMs;
                summary.nnRuns++;
                if (optInfo.available) {
                    summary.nnErrorSum += nnError;
                    summary.nnErrorRuns++;
                }

                currentStep++;
                printBenchmarkProgress(
                    config.progress,
                    currentStep,
                    totalSteps,
                    "RNN | instancja=" + entry.name
                );

                RNNResult rnnResult = RNNSolver::solve(instance);
                double rnnError = optInfo.available ? computeRelativeErrorPercent(rnnResult.bestCost, optInfo.cost) : 0.0;

                CSVWriter::appendHeuristicRow(
                    config.output_csv,
                    "RNN",
                    entry.name,
                    instance.type,
                    instance.dimension,
                    1,
                    rnnResult.bestStartVertex,
                    rnnResult.bestCost,
                    optInfo.available,
                    optInfo.cost,
                    rnnError,
                    rnnResult.timeMs
                );

                long rnnRss = getMemoryUsageOrMinusOne();
                std::cout << "  RNN"
                          << " | koszt = " << rnnResult.bestCost
                          << " | czas [ms] = " << rnnResult.timeMs
                          << " | best start = " << rnnResult.bestStartVertex;
                if (rnnRss >= 0) {
                    std::cout << " | pamiec [kB] = " << rnnRss;
                } else {
                    std::cout << " | pamiec [kB] = brak danych";
                }
                if (optInfo.available) {
                    std::cout << " | opt = " << optInfo.cost
                              << " | blad [%] = " << rnnError;
                }
                std::cout << "\n\n";

                summary.rnnTimeSum += rnnResult.timeMs;
                summary.rnnRuns++;
                if (optInfo.available) {
                    summary.rnnErrorSum += rnnError;
                    summary.rnnErrorRuns++;
                }
            }

            std::cout << "=== PODSUMOWANIE WG ROZMIARU ===\n";
            for (std::map<int, HeuristicSummary>::const_iterator it = summariesByDimension.begin();
                 it != summariesByDimension.end(); ++it) {
                int n = it->first;
                const HeuristicSummary& s = it->second;

                std::cout << "n = " << n
                          << " | RAND avg czas [ms] = " << safeAverage(s.randTimeSum, s.randRuns);
                if (s.randErrorRuns > 0) {
                    std::cout << " | RAND avg blad [%] = " << safeAverage(s.randErrorSum, s.randErrorRuns);
                }

                std::cout << " | NN avg czas [ms] = " << safeAverage(s.nnTimeSum, s.nnRuns);
                if (s.nnErrorRuns > 0) {
                    std::cout << " | NN avg blad [%] = " << safeAverage(s.nnErrorSum, s.nnErrorRuns);
                }

                std::cout << " | RNN avg czas [ms] = " << safeAverage(s.rnnTimeSum, s.rnnRuns);
                if (s.rnnErrorRuns > 0) {
                    std::cout << " | RNN avg blad [%] = " << safeAverage(s.rnnErrorSum, s.rnnErrorRuns);
                }
                std::cout << "\n";
            }

            std::cout << "Benchmark heurystyk zakonczony\n";
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
            std::cout << "Poprawnie wczytano";
        }
        else if (config.mode == "rand") {
            std::cout << "\nUruchamiam RAND...\n";
            std::cout << "Liczba losowan: " << config.rand_trials << "\n";
            std::cout << "Seed: " << config.seed << "\n";

            OptCostInfo optInfo = getOptCostFromConfig(config, instance);
            printOptInfo(optInfo);

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

            if (optInfo.available) {
                printRelativeError(result.bestCost, optInfo.cost);
            }
        }
        else if (config.mode == "nn") {
            std::cout << "\nUruchamiam NN...\n";
            std::cout << "Wierzcholek startowy: " << config.nn_start_vertex << "\n";

            OptCostInfo optInfo = getOptCostFromConfig(config, instance);
            printOptInfo(optInfo);

            NNResult result = NNSolver::solve(instance, config.nn_start_vertex);

            std::cout << "\n=== WYNIK NN ===\n";
            std::cout << "Koszt: " << result.cost << "\n";
            std::cout << "Czas [ms]: " << result.timeMs << "\n";
            std::cout << "Trasa:\n";
            std::cout << TourUtils::tourToString(result.tour) << "\n";
            printMemoryUsage();

            if (optInfo.available) {
                printRelativeError(result.cost, optInfo.cost);
            }
        }
        else if (config.mode == "rnn") {
            std::cout << "\nUruchamiam RNN...\n";

            OptCostInfo optInfo = getOptCostFromConfig(config, instance);
            printOptInfo(optInfo);

            RNNResult result = RNNSolver::solve(instance);

            std::cout << "\n=== WYNIK RNN ===\n";
            std::cout << "Najlepszy koszt: " << result.bestCost << "\n";
            std::cout << "Najlepszy start: " << result.bestStartVertex << "\n";
            std::cout << "Czas [ms]: " << result.timeMs << "\n";
            std::cout << "Najlepsza trasa:\n";
            std::cout << TourUtils::tourToString(result.bestTour) << "\n";
            printMemoryUsage();

            if (optInfo.available) {
                printRelativeError(result.bestCost, optInfo.cost);
            }
        }
        else if (config.mode == "bf") {
            std::cout << "\nUruchamiam brute force...\n";

            OptCostInfo optInfo = getOptCostFromConfig(config, instance);
            printOptInfo(optInfo);

            BruteForceResult result = BruteForceSolver::solve(instance, config.progress);

            std::cout << "\n=== WYNIK BRUTE FORCE ===\n";
            std::cout << "Najlepszy koszt: " << result.bestCost << "\n";
            std::cout << "Czas [ms]: " << result.timeMs << "\n";
            std::cout << "Sprawdzone permutacje: " << result.checkedPermutations << "\n";
            std::cout << "Najlepsza trasa:\n";
            std::cout << TourUtils::tourToString(result.bestTour) << "\n";
            printMemoryUsage();

            if (optInfo.available) {
                printRelativeError(result.bestCost, optInfo.cost);
            }
        }
        else if (config.mode == "check_opt") {
            std::cout << "\nSprawdzam trase optymalna...\n";
            std::cout << "Plik .opt.tour: " << config.opt_tour_file << "\n";

            std::vector<int> optTour = OptTourReader::loadTour(config.opt_tour_file);
            if (!TourUtils::isValidTour(optTour, instance.dimension)) {
                throw std::runtime_error("Wczytana trasa optymalna jest niepoprawna");
            }

            int optCost = TourUtils::calculateTourCost(instance, optTour);

            std::cout << "\n=== WYNIK CHECK_OPT ===\n";
            std::cout << "Koszt trasy z pliku .opt.tour: " << optCost << "\n";
            std::cout << "Trasa:\n";
            std::cout << TourUtils::tourToString(optTour) << "\n";
        }
        else {
            throw std::runtime_error("Nieznany tryb: " + config.mode);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "BLAD: " << e.what() << "\n";
        return 1;
    }

    return 0;
}