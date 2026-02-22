#pragma once
#include "message.h"
#include "json.hpp"
#include <vector>

bool limitExceeded(const std::vector<Message>& conversation, size_t limit);

std::string getConfigPath(const std::string& filename);

std::string getConfigDir();

void createConfigFileIfNotExists(const std::string& configDir, const std::string& fileTemplate);

std::string createFileIfNotExists(const std::string& chatsDir, const std::string& fileTemplate);

std::string getSystemDataPath();

std::string getChatsDir();

std::string getChatsPath(const std::string& filename);

void from_json(const nlohmann::json& j, Message& m);