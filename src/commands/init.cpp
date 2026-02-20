#include "constants.h"
#include "utils.h"
#include "commands.h"
#include <iostream>
#include <string>
#include <filesystem>

void init() {
    std::string configDir = getConfigDir();
    std::string chatsDir = getChatsDir();
    
    try {
        std::filesystem::create_directories(configDir);
        std::filesystem::create_directories(chatsDir);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return;
    }

    const char* configTemplate = R"({
        "system_prompt": "You are a helpful assistant.",
        "limit": 20,
        "max_tokens": 1024
    })";
    const char* accountsTemplate = R"({
        "accounts": []
    })";
    
    createFileIfNotExists(configDir + CONFIG_FILE, configTemplate);
    createFileIfNotExists(configDir + ACCOUNTS_FILE, accountsTemplate);

    std::cout << "\n✓ Initialization complete!\n";
    std::cout << "Next steps:\n";
    std::cout << "  1. Run 'lmcli setup' to add an account\n";
    std::cout << "  2. Run 'lmcli start' to begin chatting\n";
}