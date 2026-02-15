#include "commands.h"
#include <iostream>

void help() {
    std::cout << "lmcli - Command-line interface for LLM providers\n\n";
    std::cout << "USAGE:\n";
    std::cout << "  lmcli [COMMAND]\n\n";
    std::cout << "COMMANDS:\n";
    std::cout << "  start       Start a chat session with a selected account\n";
    std::cout << "  setup       Add a new provider account\n";
    std::cout << "  accounts    List all configured accounts\n";
    std::cout << "  help        Show this help message\n\n";
    std::cout << "EXAMPLES:\n";
    std::cout << "  lmcli              # Start chat (default)\n";
    std::cout << "  lmcli start        # Start chat\n";
    std::cout << "  lmcli setup        # Add new account\n";
    std::cout << "  lmcli accounts     # List all accounts\n\n";
}