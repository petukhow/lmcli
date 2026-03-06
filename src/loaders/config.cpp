#include "config.h"
#include "json.hpp"
#include <iostream>
#include <fstream>
#include "utils.h"

using json = nlohmann::json;

json load_config(const std::string& filepath) {
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
        std::cerr << "Config file is empty. Try 'lmcli init'.";
    }
    return parsed;
}