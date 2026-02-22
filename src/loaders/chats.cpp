#include "chatSetup.h"
#include "chats.h"
#include "utils.h"
#include "json.hpp"
#include <iostream>
#include <fstream>
#include "message.h"

using json = nlohmann::json;

void saveChat(const std::string& filePath, const std::vector<Message>& chat) {;
    std::ofstream conversation;
    json j = {{"conversation", chat}};
    try {
        conversation.exceptions(std::ofstream::failbit);
        conversation.open(filePath);
        conversation << j.dump(4);
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }
}

json loadChats(const std::string& filepath) {
    std::ifstream file(filepath);
    json parsed = {};

    try {
        parsed = json::parse(file);
    } catch (const json::parse_error& e) {
        std::cerr << "Parse error: " << e.what() << "\n";
        return {};
    }
    return parsed;
}