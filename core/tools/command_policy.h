#pragma once
#include <string>
#include "json.hpp"

bool is_confirm_required(const nlohmann::json& cfg, const std::string& cmd);

bool is_command_blacklisted(const nlohmann::json& cfg, const std::string& cmd);