#include "config.h"
#include "json.hpp"
#include "load_json.h"
#include "utils.h"
#include <fstream>
#include "constants.h"
#include <iostream>

nlohmann::json load_config(const std::string& filename) {
    const std::string full_path = get_config_path(filename);
    return load_json_file(full_path);
}

void save_config(const nlohmann::json& config) {
    std::string full_path = get_config_path(CONFIG_FILE);
    std::ofstream cfg;

    try {
        cfg.exceptions(std::ofstream::failbit);
        cfg.open(full_path);
        cfg << config.dump(4);
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }
}