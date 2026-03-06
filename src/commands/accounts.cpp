#include "commands.h"
#include "accounts.h"
#include "json.hpp"
#include <iostream>
#include "constants.h"

using json = nlohmann::json;

void accounts() {
    json accounts = load_accounts(ACCOUNTS_FILE);
    
    std::cout << "Your accounts:\n";
    for (const auto& account : accounts["accounts"]) {
        std::cout << "-- " << account["name"].get<std::string>() << "\n";
    }
}