#pragma once
#include "json.hpp"

nlohmann::json load_config(const std::string& filename);

void save_config(const nlohmann::json& config);