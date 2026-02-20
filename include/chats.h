#pragma once
#include <string>
#include "json.hpp"

void saveChat(const std::string& filepath, const nlohmann::json& chat);