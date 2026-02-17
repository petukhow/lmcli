#include "commands.h"
#include "accounts.h"
#include "json.hpp"
#include <iostream>
#include "constants.h"

using json = nlohmann::json;

void accounts() {
    json accounts = loadAccounts(ACCOUNTS_FILE);

    for (const auto& account : accounts["accounts"]) {
        std::cout << "-- " << account["name"].get<std::string>() << "\n";
    }
}