#ifndef CSVWRITER_H
#define CSVWRITER_H

#include <string>

class CSVWriter {
public:
    static void writeHeaderIfNeeded(const std::string& path);

    static void appendBruteForceRow(
        const std::string& path,
        const std::string& instanceType,
        int n,
        int instanceId,
        int bestCost,
        double timeMs,
        long long checkedPermutations
    );

    static void writeHeuristicHeaderIfNeeded(const std::string& path);

    static void appendHeuristicRow(
        const std::string& path,
        const std::string& algorithm,
        const std::string& instanceName,
        const std::string& instanceType,
        int n,
        int runId,
        int startVertex,
        int bestCost,
        bool hasOptCost,
        int optCost,
        double relativeErrorPercent,
        double timeMs
    );
};

#endif