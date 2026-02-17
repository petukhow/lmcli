#pragma once
#include "json.hpp"

nlohmann::json loadAccounts(const std::string& filepath);

void saveAccounts(const std::string& filepath, const nlohmann::json& accounts);