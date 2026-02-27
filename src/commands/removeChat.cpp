#include "chats.h"
#include "commands.h"
#include "utils.h"
#include <filesystem>
#include <iostream>
#include <string>

void removeChat() {
    std::string chatsDir = getChatsDir();
    std::string removeChatName;
    
    while (true) {
        bool isFound = false;

        std::cout << "Select a chat to remove (type '/exit' to quit):\n" ;
        printChats(chatsDir);
        std::cout << "\n> ";
        if (!std::getline(std::cin, removeChatName)) return;

        if (removeChatName == "/exit") return;

        for (const auto& file : std::filesystem::directory_iterator(chatsDir)) {
            if (file.path().stem().string() == removeChatName) {
                std::filesystem::remove(file);
                isFound = true;
                std::cout << "Removed succesfully.\n";
                break;
            }
        }
        if (!isFound) {
            std::cerr << "Chat not found.\n";
        }
    }
}