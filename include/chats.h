#pragma once
#include <string>
#include "json.hpp"
#include "message.h"
#include <vector>

void saveChat(const std::string& filepath, const std::vector<Message>& chat);

nlohmann::json loadChats(const std::string& filepath);