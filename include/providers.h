#pragma once
#include "json.hpp"

using json = nlohmann::json;

json loadProviders(const std::string& filepath);