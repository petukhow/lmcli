#pragma once
#include <string>
#include "json.hpp"

struct ToolInfo {
    std::string id;
    std::string name;
    std::string arguments;
};

std::string read_file(const std::string filename);

void to_json(nlohmann::json& j, const ToolInfo& t);

void from_json(const nlohmann::json& j, ToolInfo& t);