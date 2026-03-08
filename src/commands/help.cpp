#include "commands.h"
#include <iostream>

void help() {
    std::cout << "lmcli - Command-line interface for LLM providers\n\n";
    std::cout << "USAGE:\n";
    std::cout << "  lmcli [COMMAND] [SUBCOMMAND]\n\n";
    std::cout << "COMMANDS:\n";
    std::cout << "  start                             Start a chat session with a selected account\n";
    std::cout << "  setup                             Add a new provider account\n";
    std::cout << "  init                              Initialize config directory and files\n";
    std::cout << "  accounts                          List all configured accounts\n";
    std::cout << "  remove [account|chat|chats]       Remove account or chat(s)\n";  
    std::cout << "  config [prompt|limit|max-tokens]  Show and edit current config settings\n"; 
    std::cout << "  help                              Show this help message\n";

    std::cout << "EXAMPLES:\n";
    std::cout << "  lmcli                     # Start chat (default)\n";
    std::cout << "  lmcli start               # Start chat\n";
    std::cout << "  lmcli setup               # Add new account\n";
    std::cout << "  lmcli init                # Initialize configuration\n";
    std::cout << "  lmcli accounts            # List all accounts\n";
    std::cout << "  lmcli remove account      # Delete an account\n";
    std::cout << "  lmcli remove chat         # Delete a chat\n";
    std::cout << "  lmcli remove chats        # Delete all chats\n";
    std::cout << "  lmcli config prompt       # Check and edit system prompt\n";
    std::cout << "  lmcli config limit        # Check and edit messages limit\n";
    std::cout << "  lmcli config max-tokens   # Check and edit max tokens limit\n";
}