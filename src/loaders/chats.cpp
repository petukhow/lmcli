#include "chats.h"
#include "utils.h"
#include <iostream>
#include <fstream>

using json = nlohmann::json;

void saveChats(const std::string& filename, const json& chat) {
    std::string fullPath = getChatsPath(filename);
    std::ofstream conversation;

    try {
        conversation.exceptions(std::ofstream::failbit);
        conversation.open(fullPath);
        conversation << chat.dump(4);
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }
}
