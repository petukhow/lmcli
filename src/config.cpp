#include "config.h"
#include "json.hpp"
#include <iostream>
#include <fstream>

using json = nlohmann::json;

json loadConfig(const std::string& filepath) {
    std::ifstream file(filepath);
    std::string str;

    if (file) {
        std::ostringstream ss;
        ss << file.rdbuf(); // Read file buffer into the stringstream
        str = ss.str();     // Convert stringstream to std::string
    } else {
        // Handle file opening error
        std::cerr << "Error: Could not open the file." << std::endl;
    }
    json parsed = json::parse(str);
    return parsed;
}