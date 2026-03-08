#ifndef CONFIG_H
#define CONFIG_H

#include <string>

struct Config {
    std::string mode;
    std::string instance_file;
    std::string opt_tour_file;

    bool show_matrix = false;
    bool progress = true;

    int rand_trials = 10000;
    int nn_start_vertex = 0;

    unsigned int seed = 12345;
};

class ConfigLoader {
public:
    static Config loadFromFile(const std::string& path);
};

#endif