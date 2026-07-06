#include "commands.h"
#include <iostream>

void help() {
    std::cout << "lmcli - Command-line interface for LLM providers\n\n";
    std::cout << "USAGE:\n";
    std::cout << "  lmcli [COMMAND]\n\n";
    std::cout << "COMMANDS:\n";
    std::cout << "  start   Start a chat session (default when no command is given)\n";
    std::cout << "  init    Initialize config directory and files\n";
    std::cout << "  help    Show this help message\n\n";

    std::cout << "IN-CHAT COMMANDS:\n";
    std::cout << "  /setup    Add a new provider account\n";
    std::cout << "  /account  Switch between configured accounts\n";
    std::cout << "  /chats    Browse, open, or delete saved chats\n";
    std::cout << "  /config   Edit settings (system prompt, limits, confirmations, ...)\n";
    std::cout << "  /theme    Create, edit, or switch color themes\n";
    std::cout << "  /exit     Quit lmcli\n\n";

    std::cout << "EXAMPLES:\n";
    std::cout << "  lmcli         # Start chat (default)\n";
    std::cout << "  lmcli init    # Initialize configuration\n";
    std::cout << "  lmcli start   # Start chat explicitly\n";
}
