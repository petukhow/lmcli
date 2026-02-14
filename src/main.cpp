#include "commands.h"
#include <iostream>
#include <curl/curl.h>

int main(int argc, char* argv[]) {
    std::string lmcli = argv[0];
    if (argc == 1 && lmcli == "lmcli") {
        start();
    }
    if (argc != 2) {
        std::cout << "Unknown command.";
    } else {
    std::string command = argv[1];

    if (command == "setup") {
        setup();
    }
    else if (command == "start") {
        start();
    }
    else if (command == "accounts") {
        std::cout << "Your accounts: \n";
        accounts();
    } 
    else if (command == "help") {
        help();
    }
    else {
        std::cerr << "Unknown command: " << command << "\n";
        std::cerr << "See 'lmcli help' for available commands.\n";
        }
    }

    return 0;
}