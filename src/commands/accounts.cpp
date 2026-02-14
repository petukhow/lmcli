#include "commands.h"
#include "accounts.h"
#include "json.hpp"
#include <iostream>

using json = nlohmann::json;

void accounts() {
    json accounts = loadAccounts("accounts.json");

    for (const auto& account : accounts["accounts"]) {
        std::cout << "-- " << account["name"].get<std::string>() << "\n";
    }
}