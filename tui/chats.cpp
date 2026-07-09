#include "chats.h"
#include "constants.h"
#include "utils/utils.h"
#include "json.hpp"
#include "logging/logger.h"
#include <cmath>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "types/message.h"

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

using json = nlohmann::json;
using namespace ftxui;

bool save_chat(const std::string& filepath, const std::vector<Message>& chat) {
    std::ofstream conversation;
    const json j = {{"conversation", chat}};
    
    try {
        conversation.exceptions(std::ofstream::failbit);
        conversation.open(filepath);
        conversation << j.dump(4);
        return true;
    } catch (const std::exception& e) {
        log(LogLevel::Error, e.what());
        return false;
    }
}

std::optional<std::vector<std::filesystem::directory_entry>> list_chats(const std::string& chats_dir) {
    std::vector<std::filesystem::directory_entry> chats;

    if (!std::filesystem::exists(chats_dir)) {
        std::cerr << "\nChats directory not found. Try 'lmcli init'.\n";
        return chats;
    }

    if (!std::filesystem::is_empty(chats_dir)) {
        for (const auto& file : std::filesystem::directory_iterator(chats_dir)) {
            chats.push_back(file);
        }
    } else {
        return std::nullopt;
    }
    return chats;
}


std::string create_chat(const std::string& chats_dir) {
    size_t files_amount = std::distance(std::filesystem::directory_iterator(chats_dir),
    std::filesystem::directory_iterator{});

    std::string full_chat_name = chats_dir + "chat #" + std::to_string(files_amount) + ".json";

    std::string chat_path = create_file_if_not_exists(full_chat_name, CHAT_DEFAULT);
    return chat_path;
}