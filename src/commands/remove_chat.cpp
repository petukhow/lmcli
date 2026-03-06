#include "chats.h"
#include "commands.h"
#include "utils.h"
#include <filesystem>
#include <iostream>
#include <string>

void remove_chat() {
    const std::string chats_dir = get_chats_dir();
    const auto chats = store_chats(chats_dir);
    std::string remove_chat_name;
    
    while (true) {
        bool is_found = false;

        std::cout << "Select a chat to remove (type '/exit' to quit):\n";
        print_chats(chats);
        std::cout << "\n> ";
        if (!std::getline(std::cin, remove_chat_name)) return;

        if (remove_chat_name == "/exit") return;

        for (const auto& file : std::filesystem::directory_iterator(chats_dir)) {
            if (file.path().stem().string() == remove_chat_name) {
                std::filesystem::remove(file);
                is_found = true;
                std::cout << "Removed succesfully.\n";
                break;
            }
        }
        if (!is_found) {
            std::cerr << "Chat not found.\n";
        }
    }
}