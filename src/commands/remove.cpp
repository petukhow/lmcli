#include "commands.h"
#include "accounts.h"
#include "constants.h"
#include "json.hpp"
#include <string>
#include <iostream>

using json = nlohmann::json;

void removeAccount() {
    std::string removedName;
    bool isFound = false;
    json accounts = loadAccounts(ACCOUNTS_FILE);

    if (!accounts["accounts"].empty()) {
        std::cout << "Select an account to remove (type '/exit' to quit):\n";
        for (const auto& account : accounts["accounts"]) {
            std::cout << "-- " << account["name"].get<std::string>() << "\n";
        }
    } else {
        std::cerr << "No accounts configured.";
    }

    std::cout << "> ";
    std::getline(std::cin, removedName);

    if (removedName == "/exit") return;
    
    for (size_t i = 0; i < accounts["accounts"].size(); ++i) {
        if (accounts["accounts"][i]["name"].get<std::string>() == removedName) {
            isFound = true;
            accounts["accounts"].erase(i);
            saveAccounts(ACCOUNTS_FILE, accounts);
            std::cout << "Removed successfully\n";
            break;
        }
    }
    if (!isFound) {
        std::cerr << "Account not found.\n";
    }
}
