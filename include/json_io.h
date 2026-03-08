#pragma once
#include <string>
#include "json.hpp"

void save_json(const nlohmann::json& filepath);

nlohmann::json load_json(const std::string& filepath);