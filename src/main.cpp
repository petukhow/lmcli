#include "commands.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc == 1) {
        start();
        return 0;
    }
    
    if (argc > 3) {
        std::cerr << "Usage: lmcli [COMMAND] [SUBCOMMAND]\n";
        std::cerr << "See 'lmcli help' for available commands and subcommands.\n";
        return 1;
    }

    std::string command = argv[1];

    if (command == "setup") {
        setup();
    }
    else if (command == "start") {
        start();
    }
    else if (command == "accounts") {
        accounts();
    } 
    else if (command == "help") {
        help();
    }
    else if (command == "init") {
        init();
    }
    else if (command == "remove") {
        std::string subcommand;
        if (argc == 3) {
            subcommand = argv[2];
        }
        if (subcommand == "chat") {
            removeChat();
        }
        else if (subcommand == "account") {
            removeAccount();
        }
        else {
            std::cerr << "Undefined command. Usage: lmcli remove [account|chat]\n";
        }
    }

    else {
        std::cerr << "Unknown command: " << command << "\n";
        std::cerr << "See 'lmcli help' for available commands.\n";
        return 1;
    }

    return 0;
}