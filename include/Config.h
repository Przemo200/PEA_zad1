#ifndef CONFIG_H
#define CONFIG_H

#include <string>

struct Config {
    std::string mode;
    std::string instance_file;
    std::string opt_tour_file;
    std::string output_csv = "results/results.csv";
    std::string generated_type = "ATSP";
    std::string heuristics_list_file;

    bool show_matrix = false;
    bool progress = true;

    int rand_trials = 10000;
    int rand_repeats = 5;
    int nn_start_vertex = 0;
    int single_opt_cost = -1;

    int bf_min_n = 6;
    int bf_max_n = 8;
    int bf_instances_per_size = 3;

    int weight_min = 1;
    int weight_max = 100;

    unsigned int seed = 12345;
};

class ConfigLoader {
public:
    static Config loadFromFile(const std::string& path);

private:
    static void validate(const Config& config);
};

#endif