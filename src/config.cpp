#include "config.h"
#include "json.hpp"
#include <iostream>
#include <fstream>
#include "utils.h"

using json = nlohmann::json;

json loadConfig(const std::string& filepath) {
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
    parsed = json::parse(str);
    return parsed;
}