#include "accounts.h"
#include "json.hpp"
#include <exception>
#include <iostream>
#include <fstream>
#include "utils.h"
#include "load_json.h"

nlohmann::json load_accounts(const std::string& filepath) {
    const std::string fullpath = get_config_path(filepath);
    return load_json_file(fullpath);
}

void save_accounts(const std::string& filename, const nlohmann::json& accounts) {
    std::string full_path = get_config_path(filename);
    std::ofstream accounts_list;

    try {
        accounts_list.exceptions(std::ofstream::failbit);
        accounts_list.open(full_path);
        accounts_list << accounts.dump(4);
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }
}