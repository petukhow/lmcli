#pragma once
#include <string>
#include "json.hpp"
#include "message.h"
#include <vector>

void saveChat(const std::string& filepath, const std::vector<Message>& chat);

nlohmann::json loadChats(const std::string& filepath);

std::string setupChat();

void printChats(const std::string chatsDir);

std::string createChat(const std::string& chatsDir);

std::string continueChat(const std::string& chatsDir, const std::string& chatName);