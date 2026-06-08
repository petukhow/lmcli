#include <string>
#include <iostream>

bool ask_confirmation(const std::string& cmd) {
    std::string answ;
    std::cerr << "\nModel wants to execute bash command: " << cmd << "\n";
    std::cerr << "Continue? (y/n) ";

    std::getline(std::cin, answ);
    
    if (!answ.empty() && std::tolower(answ[0]) == 'y') {
        return true;
    }
    return false;
}