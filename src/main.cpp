#include "commands.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc == 1) {
        start();
        return 0;
    }
    
    if (argc != 2) {
        std::cerr << "Usage: lmcli [COMMAND]\n";
        std::cerr << "See 'lmcli help' for available commands.\n";
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
        std::cout << "Your accounts:\n";
        accounts();
    } 
    else if (command == "help") {
        help();
    }
    else {
        std::cerr << "Unknown command: " << command << "\n";
        std::cerr << "See 'lmcli help' for available commands.\n";
        return 1;
    }

    return 0;
}