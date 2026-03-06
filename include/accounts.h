#pragma once
#include "json.hpp"

nlohmann::json load_accounts(const std::string& filepath);

void save_accounts(const std::string& filepath, const nlohmann::json& accounts);