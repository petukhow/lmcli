#include <filesystem>
#include "utils.h"
#include <iostream>

std::string chatSetup() {
    std::string chatPath;
    std::string chatName;
    std::string fullChatName;
    std::string chatsDir = getChatsDir();
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

    std::cout << "Enter chat's name to continue (/exit to leave, /new to create): \n";

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, chatName);

        if (chatName == "/exit") break;

        else if (chatName == "/new") {
            std::cout << "Enter chat's name (Leave empty for default: 'chat #...'): \n";
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
                chatPath = createFileIfNotExists(fullChatName, chatTemplate);
                return chatPath;
            } else {
                chatPath = createFileIfNotExists(fullChatName, chatTemplate);
                return chatPath;
            }
                
        } else {
            fullChatName = chatsDir + chatName + ".json"; 
            if (std::filesystem::exists(fullChatName)) {
                return fullChatName;
            } else {
                std::cerr << "No chat with given name found. Try again.\n";
                continue;
            }
        }
    }
    return fullChatName;
}