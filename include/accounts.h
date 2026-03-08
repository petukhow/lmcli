#pragma once
#include "json.hpp"

nlohmann::json load_accounts(const std::string& filename);

void save_accounts(const nlohmann::json& accounts);