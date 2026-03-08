#include "accounts.h"
#include "constants.h"
#include "json.hpp"
#include <exception>
#include <iostream>
#include <fstream>
#include "utils.h"
#include "load_json.h"

nlohmann::json load_accounts(const std::string& filename) {
    const std::string full_path = get_config_path(filename);
    return load_json_file(full_path);
}

void save_accounts(const nlohmann::json& accounts) {
    std::string full_path = get_config_path(ACCOUNTS_FILE);
    std::ofstream accs;

    try {
        accs.exceptions(std::ofstream::failbit);
        accs.open(full_path);
        accs << accounts.dump(4);
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }
}