#include <string>
#include <iostream>
#include "json.hpp"
#include <fstream>
#include "json_io.h"

nlohmann::json load_json(const std::string& filepath) {
    std::ifstream file(filepath);
    std::string str;
    nlohmann::json parsed = {};

    if (file) {
        std::ostringstream ss;
        ss << file.rdbuf(); // Read file buffer into the stringstream
        str = ss.str();     // Convert stringstream to std::string
    } else {
        // Handle file opening error
        std::cerr << "Could not open file: " + filepath << "\n";
        return parsed;
    }

    try {
        parsed = nlohmann::json::parse(str);
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Parse error: " << e.what() << "\n";
        return {};
    }   
    if (parsed.empty()) {
        std::cerr << "File " + filepath << " is empty. Try 'lmcli init'.";
    }
    return parsed;
}

void save_json(const nlohmann::json& filepath) {
    std::ofstream accs;
    try {
        accs.exceptions(std::ofstream::failbit);
        accs.open(filepath);
        accs << filepath.dump(4);
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }
}