#include "chats.h"
#include "utils.h"
#include "json.hpp"
#include <iostream>
#include <fstream>
#include "message.h"

using json = nlohmann::json;

std::string setupChat() {
    std::string chatPath; // path to the chat file
    std::string chatName; // chat name with extension
    std::string fullChatName; // full path to the chat file (chatsDir + chatName)
    std::string chatsDir = getChatsDir(); // path to the chats directory
    
    printChats(chatsDir); 

    std::cout << "Enter chat's name to continue (/exit to leave, /new to create): \n";
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, chatName);

        if (chatName == "/exit") {
            break;
        } else if (chatName == "/new") {
            fullChatName = createChat(chatsDir);
            if (fullChatName != "") {
                break;
            } else {
                std::cerr << "Failed to create a new chat. Try again.\n";
            }
        } else {
            fullChatName = continueChat(chatsDir, chatName);
            if (fullChatName != "") {
                break;
            } else {
                std::cerr << "Chat not found. Try again.\n";
            }
        }
    }
    return fullChatName;
}

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

void printChats(const std::string chatsDir) {
    bool empty = std::filesystem::is_empty(chatsDir);
    
    if (!empty) {
        std::cout << "Chats: \n";
        for (const auto& file : std::filesystem::directory_iterator(chatsDir)) {
            std::cout << "-- ";
            std::cout << file.path().stem().string() << "\n";
        }  
    } else {
        std::cerr << "No chats yet. Start a new chat to create one.\n";
    }
}

std::string createChat(const std::string& chatsDir) {
    std::string chatName; // chat name with extension
    std::string fullChatName; // full path to the chat file (chatsDir + chatName)
    std::string chatPath; // path to the chat file

    std::cout << "Enter chat's name (leave empty for default: 'chat #...'): \n";
    std::cout << "> ";
    std::getline(std::cin, chatName);

    fullChatName = chatsDir + chatName + ".json"; 

    const char* chatTemplate = R"({
        "conversation": []
    })";

    size_t filesAmount = std::distance(std::filesystem::directory_iterator(chatsDir),
    std::filesystem::directory_iterator{});
    if (chatName == "") {
        fullChatName = chatsDir + "chat #" + std::to_string(filesAmount) + ".json";
    }
    chatPath = createFileIfNotExists(fullChatName, chatTemplate);
    return chatPath;
}

std::string continueChat(const std::string& chatsDir, const std::string& chatName) {
    std::string fullChatName = chatsDir + chatName + ".json"; 
    if (std::filesystem::exists(fullChatName)) {
        return fullChatName;
    } else {
        std::cerr << "No chat with given name found. Try again.\n";
        return "";
    }
}