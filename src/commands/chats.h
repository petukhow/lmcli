#pragma once
#include <string>
#include "json.hpp"
#include "types/message.h"
#include <vector>

void save_chat(const std::string& filepath, const std::vector<Message>& chat);

nlohmann::json load_chats(const std::string& filepath);

std::string setup_chat();

std::vector<std::filesystem::directory_entry> store_chats(const std::string& chats_dir);

void print_chats(const std::vector<std::filesystem::directory_entry>& chats);

std::string create_chat(const std::string& chats_dir);

std::string continue_chat(const std::string& chats_dir, const std::string& chat_name,
    const std::vector<std::filesystem::directory_entry>& chats);