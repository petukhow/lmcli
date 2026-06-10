#pragma once
#include <string>
#include "json.hpp"

void save_json(const std::string& filepath, const nlohmann::json& data);

std::optional<nlohmann::json> load_json(const std::string& filepath);