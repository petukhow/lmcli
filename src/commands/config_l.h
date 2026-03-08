#pragma once
#include "json.hpp"

void edit_system_prompt();

void edit_limit();

void edit_max_tokens();

void config();

void save_config(const nlohmann::json& config);