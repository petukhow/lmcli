#pragma once
#include <string>
#include "types/message.h"
#include <vector>

bool save_chat(const std::string& filepath, const std::vector<Message>& chat);

std::vector<std::filesystem::directory_entry> list_chats(const std::string& chats_dir);

std::string create_chat(const std::string& chats_dir);
