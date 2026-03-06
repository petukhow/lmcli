#include "accounts.h"
#include "json.hpp"
#include <exception>
#include <iostream>
#include <fstream>
#include "utils.h"

using json = nlohmann::json;

json load_accounts(const std::string& filepath) {
    std::string full_path = get_config_path(filepath);
    std::ifstream file(full_path);
    std::string str;
    json parsed = {};

    if (file) {
        std::ostringstream ss;
        ss << file.rdbuf(); // Read file buffer into the stringstream
        str = ss.str();     // Convert stringstream to std::string
    } else {
        // Handle file opening error
        std::cerr << "Error: Could not open file: " << full_path << "\n";
        return parsed;
    }
    try {
        parsed = json::parse(str);
    } catch (const json::parse_error& e) {
        std::cerr << "Parse error: " << e.what() << "\n";
        return {};
    }
    if (parsed.empty()) {
        std::cerr << "Accounts file is empty. Try 'lmcli init'.";
    }
    return parsed;
}

void save_accounts(const std::string& filename, const json& accounts) {
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