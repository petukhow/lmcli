#pragma once
#include <string>
#include "json.hpp"
#include <optional>

struct ToolInfo {
    std::string id;
    std::string name;
    std::string arguments;
    std::optional<std::string> thought_signature;
};

void to_json(nlohmann::json& j, const ToolInfo& t);

void from_json(const nlohmann::json& j, ToolInfo& t);