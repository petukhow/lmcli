#pragma once
#include "types/theme.h"
#include "json.hpp"
#include <string>

Theme load_theme(const std::string& theme_name);

nlohmann::json load_themes_file();

void save_themes_file(const nlohmann::json& themes_data);

bool theme_name_exists(const nlohmann::json& themes_data, const std::string& theme_name);
