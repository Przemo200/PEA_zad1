#include "Config.h"
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
            config.mode = value;
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

    return config;
}