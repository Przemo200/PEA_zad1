#include "../include/CSVWriter.h"
#include <filesystem>
#include <fstream>
#include <stdexcept>

void CSVWriter::writeHeaderIfNeeded(const std::string& path) {
    bool needHeader = true;

    if (std::filesystem::exists(path)) {
        std::ifstream in(path); // jesli istnieje i niepusty to nie trzeba headera
        if (in.good() && in.peek() != std::ifstream::traits_type::eof()) {
            needHeader = false;
        }
    }

    if (needHeader) {
        std::filesystem::path p(path);
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path());
        }

        std::ofstream out(path, std::ios::app);
        if (!out.is_open()) {
            throw std::runtime_error("Nie mozna otworzyc pliku CSV: " + path);
        }

        // naglowek brute force
        out << "algorithm,instance_type,n,instance_id,best_cost,time_ms,checked_permutations\n";
    }
}

void CSVWriter::appendBruteForceRow(
    const std::string& path,
    const std::string& instanceType,
    int n,
    int instanceId,
    int bestCost,
    double timeMs,
    long long checkedPermutations
) {
    // tryb dopisywania i dopisuje jeden wiersz
    std::ofstream out(path, std::ios::app);
    if (!out.is_open()) {
        throw std::runtime_error("Nie mozna dopisac do pliku CSV: " + path);
    }

    out << "BF" << ","
        << instanceType << ","
        << n << ","
        << instanceId << ","
        << bestCost << ","
        << timeMs << ","
        << checkedPermutations << "\n";
}

// odpowiednik naglowka dla heurystyk
void CSVWriter::writeHeuristicHeaderIfNeeded(const std::string& path) {
    bool needHeader = true;

    if (std::filesystem::exists(path)) {
        std::ifstream in(path);
        if (in.good() && in.peek() != std::ifstream::traits_type::eof()) {
            needHeader = false;
        }
    }

    if (needHeader) {
        std::filesystem::path p(path);
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path());
        }

        std::ofstream out(path, std::ios::app);
        if (!out.is_open()) {
            throw std::runtime_error("Nie mozna otworzyc pliku CSV: " + path);
        }

        out << "algorithm,instance_name,instance_type,n,run_id,start_vertex,best_cost,opt_cost,relative_error_percent,time_ms\n";
    }
}

// dodanie wiersza do csv heurystyk
void CSVWriter::appendHeuristicRow(
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
) {
    std::ofstream out(path, std::ios::app);
    if (!out.is_open()) {
        throw std::runtime_error("Nie mozna dopisac do pliku CSV: " + path);
    }

    out << algorithm << ","
        << instanceName << ","
        << instanceType << ","
        << n << ","
        << runId << ","
        << startVertex << ","
        << bestCost << ",";

    // jak opt znane to opt cost i error a jak nie to te pola puste
    if (hasOptCost) {
        out << optCost << "," << relativeErrorPercent;
    } else {
        out << "," << "";
    }

    out << "," << timeMs << "\n";
}