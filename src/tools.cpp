#include "tools.h"
#include <sstream>
#include <iostream>
#include <string>
#include <fstream>

std::string read_file(const std::string filename) {
    std::ifstream file(filename); 

    if (!file.is_open()) {
        std::cerr << "Error: Could not open the file." << std::endl;
        return "";
    }

    std::ostringstream ss;
    ss << file.rdbuf();

    return ss.str();
}

void to_json(nlohmann::json& j, const ToolInfo& t) {
    j = nlohmann::json{{"id", t.id}, {"name", t.name}, {"arguments", t.arguments}};
}

void from_json(const nlohmann::json& j, ToolInfo& t) {
    j.at("id").get_to(t.id);
    j.at("name").get_to(t.name);
    j.at("arguments").get_to(t.arguments);
}