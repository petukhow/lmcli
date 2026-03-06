#include "commands.h"
#include "accounts.h"
#include "constants.h"
#include "json.hpp"
#include <string>
#include <iostream>

using json = nlohmann::json;

void remove_account() {
    std::string remove_account_name;
    bool is_found = false;
    json accounts = load_accounts(ACCOUNTS_FILE);

    while (true) {
        std::cout << "Select an account to remove (type '/exit' to quit):\n";
        for (const auto& account : accounts["accounts"]) {
            std::cout << "-- " << account["name"].get<std::string>() << "\n";
        }

        std::cout << "> ";
        if (!std::getline(std::cin, remove_account_name)) return;

        if (remove_account_name == "/exit") return;

        for (size_t i = 0; i < accounts["accounts"].size(); ++i) {
            if (accounts["accounts"][i]["name"].get<std::string>() == remove_account_name) {
                is_found = true;
                accounts["accounts"].erase(i);
                save_accounts(ACCOUNTS_FILE, accounts);
                std::cout << "Removed successfully.\n";
                break;
            }
        }
        if (!is_found) {
            std::cerr << "Account not found.\n";
        }
    }
}
