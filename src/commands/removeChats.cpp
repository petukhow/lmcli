#include "commands.h"
#include "utils.h"
#include <cctype>
#include <iostream>
#include <filesystem>
#include <string>

void removeChats() {
    std::string chatsDir = getChatsDir();
    std::string userAnswer;

    std::cerr << "WARNING: it will remove all chats.\n";
        while (true) {
        std::cerr << "Do you want to continue (y/n)? ";
        if (!std::getline(std::cin, userAnswer)) break;
        if (userAnswer.empty()) continue;

        if (std::tolower(userAnswer[0]) == 'y') {
            for (const auto& chat : std::filesystem::directory_iterator(chatsDir)) {
                std::filesystem::remove(chat);  
            } 
            std::cout << "All chats removed successfully.\n";
            break;
            
        } else if (std::tolower(userAnswer[0]) == 'n') break; 
    }
    
}