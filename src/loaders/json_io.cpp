#include <string>
#include "json.hpp"
#include "logging/logger.h"
#include <fstream>
#include "json_io.h"

std::optional<nlohmann::json> load_json(const std::string& filepath) {
    std::ifstream file(filepath);
    std::string str;
    nlohmann::json parsed = {};

    if (file) {
        std::ostringstream ss;
        ss << file.rdbuf(); // Read file buffer into the stringstream
        str = ss.str();     // Convert stringstream to std::string
    } else {
        // Handle file opening error
        log(LogLevel::Error, "Could not open file " + filepath); 
        return std::nullopt;
    }

    try {
        parsed = nlohmann::json::parse(str);
        log(LogLevel::Debug, "Parsing " + str);
    } catch (const nlohmann::json::parse_error& e) {
        log(LogLevel::Error, "Parse error: " + std::string(e.what()));
        return std::nullopt;
    }   
    if (parsed.empty()) {
        log(LogLevel::Error, filepath + " is empty");
        return std::nullopt;
    }
    return parsed;
}

void save_json(const std::string& filepath, const nlohmann::json& data) {
    std::ofstream accs;
    try {
        accs.exceptions(std::ofstream::failbit);
        accs.open(filepath);
        accs << data.dump(4);
        log(LogLevel::Info, filepath + " saved");
    } catch (const std::exception& e) {
        log(LogLevel::Error, "Failed to save in file " + filepath + ":" + std::string(e.what()));
    }
}