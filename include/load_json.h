#pragma once
#include <string>
#include "json.hpp"

nlohmann::json load_json_file(const std::string& filepath);