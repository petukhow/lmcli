#include "constants.h"
#include "utils/utils.h"
#include "commands.h"
#include <iostream>
#include <string>
#include <filesystem>

void init() {
    std::string config_dir = get_config_dir();
    std::string chats_dir = get_chats_dir();
    
    try {
        std::filesystem::create_directories(config_dir);
        std::filesystem::create_directories(chats_dir);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return;
    }

    const char* config_template = R"({
        "system_prompt": "You are a helpful assistant.",
        "limit": 20,
        "max_tokens": 1024
    })";
    const char* accounts_template = R"({
        "accounts": []
    })";

    create_config_file_if_not_exists(config_dir + CONFIG_FILE, config_template);
    create_config_file_if_not_exists(config_dir + ACCOUNTS_FILE, accounts_template);

    std::cout << "\n✓ Initialization complete!\n";
    std::cout << "Next steps:\n";
    std::cout << "  1. Run 'lmcli setup' to add an account\n";
    std::cout << "  2. Run 'lmcli start' to begin chatting\n";
}