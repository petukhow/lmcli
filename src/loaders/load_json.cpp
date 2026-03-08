#include <string>
#include <iostream>
#include "json.hpp"
#include <fstream>

nlohmann::json load_json_file(const std::string& filepath) {
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