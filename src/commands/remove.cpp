#include "chats.h"
#include "commands.h"
#include "utils.h"
#include <filesystem>
#include <iostream>
#include <string>
#include "constants.h"
#include "accounts.h"

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

void remove_chats() {
    std::string chats_dir = get_chats_dir();
    std::string user_answer;

    std::cerr << "WARNING: it will remove all chats.\n";
        while (true) {
        std::cerr << "Do you want to continue (y/n)? ";
        if (!std::getline(std::cin, user_answer)) break;
        if (user_answer.empty()) continue;

        if (std::tolower(user_answer[0]) == 'y') {
            for (const auto& chat : std::filesystem::directory_iterator(chats_dir)) {
                std::filesystem::remove(chat);  
            } 
            std::cout << "All chats removed successfully.\n";
            break;
            
        } else if (std::tolower(user_answer[0]) == 'n') break; 
    }
}

void remove_account() {
    std::string remove_account_name;
    bool is_found = false;
    nlohmann::json accounts = load_accounts(ACCOUNTS_FILE);

    while (true) {
        is_found = false;
        std::cout << "Select an account to remove (type '/exit' to quit):\n";
        for (const auto& account : accounts["accounts"]) {
            std::cout << "-- " << account["name"].get<std::string>() << "\n";
        }

        std::cout << "> ";
        if (!std::getline(std::cin, remove_account_name)) return;

        if (remove_account_name == "/exit") return;

        for (size_t i = 0; i < accounts["accounts"].size(); ++i) {
            if (accounts["accounts"][i]["name"].get<std::string>() == remove_account_name) {
                is_found = true;
                accounts["accounts"].erase(i);
                save_accounts();
                std::cout << "Removed successfully.\n";
                break;
            }
        }
        if (!is_found) {
            std::cerr << "Account not found.\n";
        }
    }
}

void remove(const std::string& subcommand) {
    if (subcommand == "chat") {
        remove_chat();
    }
    else if (subcommand == "account") {
        remove_account();
    }
    else if (subcommand == "chats") {
        remove_chats();
    }
    else std::cerr << "Usage: lmcli remove [account|chat|chats]\n";
}