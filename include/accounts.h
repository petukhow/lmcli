#pragma once
#include "json.hpp"

using json = nlohmann::json;

json loadAccounts(const std::string& filepath);