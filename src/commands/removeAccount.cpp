#include "commands.h"
#include "accounts.h"
#include "constants.h"
#include "json.hpp"
#include <string>
#include <iostream>

using json = nlohmann::json;

void removeAccount() {
    std::string removeAccountName;
    bool isFound = false;
    json accounts = loadAccounts(ACCOUNTS_FILE);

    while (true) {
        std::cout << "Select an account to remove (type '/exit' to quit):\n";
        for (const auto& account : accounts["accounts"]) {
            std::cout << "-- " << account["name"].get<std::string>() << "\n";
        }

        std::cout << "> ";
        if (!std::getline(std::cin, removeAccountName)) return;

        if (removeAccountName == "/exit") return;
        
        for (size_t i = 0; i < accounts["accounts"].size(); ++i) {
            if (accounts["accounts"][i]["name"].get<std::string>() == removeAccountName) {
                isFound = true;
                accounts["accounts"].erase(i);
                saveAccounts(ACCOUNTS_FILE, accounts);
                std::cout << "Removed successfully.\n";
                break;
            }
        }
        if (!isFound) {
            std::cerr << "Account not found.\n";
        }
    }
}
