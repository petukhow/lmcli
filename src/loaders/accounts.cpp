#include "accounts.h"
#include "json.hpp"
#include <exception>
#include <iostream>
#include <fstream>
#include "utils.h"

using json = nlohmann::json;

json loadAccounts(const std::string& filepath) {
    std::string fullPath = getConfigPath(filepath);
    std::ifstream file(fullPath);
    std::string str;
    json parsed = {};

    if (file) {
        std::ostringstream ss;
        ss << file.rdbuf(); // Read file buffer into the stringstream
        str = ss.str();     // Convert stringstream to std::string
    } else {
        // Handle file opening error
        std::cerr << "Error: Could not open file: " << fullPath << "\n";
        return parsed;
    }
    try {
        parsed = json::parse(str);
    } catch (const json::parse_error& e) {
        std::cerr << "parse error: " << e.what() << "\n";
        return {};
}
    return parsed;
}

void saveAccounts(const std::string& filename, const json& accounts) {
    std::string fullPath = getConfigPath(filename);
    std::ofstream accountsList;

    try {
        accountsList.exceptions(std::ofstream::failbit);
        accountsList.open(fullPath);
        accountsList << accounts.dump(4);
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }
}