#include "providers.h"
#include "constants.h"
#include "json.hpp"
#include <iostream>
#include <fstream>
#include "utils.h"

using json = nlohmann::json;

json loadProviders() {
    std::string fullPath = getSystemDataPath() + PROVIDERS_FILE;
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
        std::cerr << "Parse error: " << e.what() << "\n";
        return {};
}
    return parsed;
}