#include "../include/Config.h"
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

namespace {
    std::string trim(const std::string& s) {
        size_t start = 0;
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
            start++;
        }

        size_t end = s.size();
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
            end--;
        }

        return s.substr(start, end - start);
    }

    std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return s;
    }

    bool toBool(const std::string& value) {
        std::string v = toLower(trim(value));
        return (v == "true" || v == "1" || v == "yes" || v == "on");
    }
}

Config ConfigLoader::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Nie mozna otworzyc pliku konfiguracyjnego: " + path);
    }

    Config config;
    std::string line;

    while (std::getline(file, line)) {
        line = trim(line);

        if (line.empty() || line[0] == '#') {
            continue;
        }

        size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }

        std::string key = toLower(trim(line.substr(0, eq)));
        std::string value = trim(line.substr(eq + 1));

        if (key == "mode") {
            config.mode = toLower(value);
        } else if (key == "instance_file") {
            config.instance_file = value;
        } else if (key == "opt_tour_file") {
            config.opt_tour_file = value;
        } else if (key == "output_csv") {
            config.output_csv = value;
        } else if (key == "generated_type") {
            config.generated_type = value;
        } else if (key == "heuristics_list_file") {
            config.heuristics_list_file = value;
        } else if (key == "show_matrix") {
            config.show_matrix = toBool(value);
        } else if (key == "progress") {
            config.progress = toBool(value);
        } else if (key == "rand_trials") {
            config.rand_trials = std::stoi(value);
        } else if (key == "rand_repeats") {
            config.rand_repeats = std::stoi(value);
        } else if (key == "nn_start_vertex") {
            config.nn_start_vertex = std::stoi(value);
        } else if (key == "bf_min_n") {
            config.bf_min_n = std::stoi(value);
        } else if (key == "bf_max_n") {
            config.bf_max_n = std::stoi(value);
        } else if (key == "bf_instances_per_size") {
            config.bf_instances_per_size = std::stoi(value);
        } else if (key == "weight_min") {
            config.weight_min = std::stoi(value);
        } else if (key == "weight_max") {
            config.weight_max = std::stoi(value);
        } else if (key == "seed") {
            config.seed = static_cast<unsigned int>(std::stoul(value));
        } else if (key == "single_opt_cost") {
            config.single_opt_cost = std::stoi(value);
        }
    }

    validate(config);
    return config;
}

void ConfigLoader::validate(const Config& config) {
    if (config.mode.empty()) {
        throw std::runtime_error("Brak pola mode w pliku konfiguracyjnym.");
    }

    if (config.rand_trials <= 0) {
        throw std::runtime_error("rand_trials musi byc > 0.");
    }

    if (config.rand_repeats <= 0) {
        throw std::runtime_error("rand_repeats musi byc > 0.");
    }

    if (config.bf_min_n <= 0 || config.bf_max_n <= 0) {
        throw std::runtime_error("bf_min_n i bf_max_n musza byc > 0.");
    }

    if (config.bf_min_n > config.bf_max_n) {
        throw std::runtime_error("bf_min_n nie moze byc wieksze od bf_max_n.");
    }

    if (config.bf_instances_per_size <= 0) {
        throw std::runtime_error("bf_instances_per_size musi byc > 0.");
    }

    if (config.weight_min > config.weight_max) {
        throw std::runtime_error("weight_min nie moze byc wieksze od weight_max.");
    }

    if (config.mode == "test_read" || config.mode == "rand" || config.mode == "nn" ||
        config.mode == "rnn" || config.mode == "bf" || config.mode == "check_opt") {
        if (config.instance_file.empty()) {
            throw std::runtime_error("Dla tego trybu wymagane jest instance_file.");
        }
    }

    if (config.mode == "check_opt" && config.opt_tour_file.empty()) {
        throw std::runtime_error("Dla trybu check_opt wymagane jest opt_tour_file.");
    }

    if (config.mode == "benchmark_heuristics" && config.heuristics_list_file.empty()) {
        throw std::runtime_error("Dla trybu benchmark_heuristics wymagane jest heuristics_list_file.");
    }
}