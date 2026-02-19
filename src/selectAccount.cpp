#include "selectAccount.h"
#include "provider.h"
#include "json.hpp"
#include <iostream>

using json = nlohmann::json;

std::unique_ptr<Provider> selectAccount(const json& accounts, const json& config) {
    std::string providerName;
    std::unique_ptr<Provider> provider = nullptr;

    // Check if accounts array exists and is not empty
    if (accounts.contains("accounts") && !accounts["accounts"].empty()) {
        while (true) {
            std::cout << "Select an account from the list below (type '/exit' to quit):\n";

            for (const auto& account : accounts["accounts"]) {
                std::cout << "-- " << account["name"].get<std::string>() << "\n";
            }   

            std::cout << "> ";
            std::getline(std::cin, providerName);
            if (providerName == "/exit") break;

            for (const auto& acc : accounts["accounts"]) {
                if (acc["name"].get<std::string>() == providerName) {
                    provider = Provider::create(acc, config);
                    break;
                }
            }
            
            if (provider == nullptr) {
                std::cout << "Unexpected input. Try again.\n";
                continue;
            } else {
                break;
                }
        }     
    } else {
        std::cerr << "You have broken accounts.json. Try 'lmcli init'.";
    }
    return provider;
}