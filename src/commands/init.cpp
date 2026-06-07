#include "constants.h"
#include "logging/logger.h"
#include "utils/utils.h"
#include "commands.h"
#include <iostream>
#include <string>
#include <filesystem>

void init() {
    std::string config_dir = get_config_dir();
    std::string chats_dir = get_chats_dir();
    std::string log_dir = get_log_dir();
    
    try {
        std::filesystem::create_directories(config_dir);
        std::filesystem::create_directories(chats_dir);
        std::filesystem::create_directories(log_dir);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << "\n";
        log(LogLevel::Error, e.what());
        return;
    }

    create_config_file_if_not_exists(config_dir + CONFIG_FILE, CONFIG_DEFAULT);
    create_config_file_if_not_exists(config_dir + ACCOUNTS_FILE, ACCOUNTS_DEFAULT);

    std::cout << "\n✓ Initialization complete!\n";
    std::cout << "Next steps:\n";
    std::cout << "  1. Run 'lmcli setup' to add an account\n";
    std::cout << "  2. Run 'lmcli start' to begin chatting\n";
}