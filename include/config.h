#pragma once
#include "json.hpp"

using json = nlohmann::json;

json loadConfig(const std::string& filepath);