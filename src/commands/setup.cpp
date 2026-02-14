#include "commands.h"
#include "accounts.h"
#include "json.hpp"
#include "config.h"
#include <iostream>

using json = nlohmann::json;

void setup(json& providers) {
    std::string providerName; // user's choose
    json accounts = loadAccounts("../accounts.json");
    json newAccount; // user's new account

    // config settings
    std::string defaultModel; 
    std::string defaultUrl;
    std::string type;

    // user's data
    std::string accountName;
    std::string apiKey;
    std::string userModel;

    std::cout << "Pick a provider or /exit to leave:" << "\n";

    for (const auto& provider : providers["providers"]) {
        std::cout << "-- " << provider["name"].get<std::string>() << "\n";
    }

    std::cout << "> ";
    std::getline(std::cin, providerName);

    for (const auto& provider : providers["providers"]) {
        if (provider["name"].get<std::string>() == providerName) {
            type = provider["type"].get<std::string>();
            defaultUrl = provider["default_api_url"].get<std::string>();
            defaultModel = provider["default_model"].get<std::string>();

            std::cout << "Enter account name: ";
            std::getline(std::cin, accountName);

            std::cout << "Enter API key: ";
            std::getline(std::cin, apiKey);

            std::cout << "Enter model (default: " << defaultModel << "): ";
            std::getline(std::cin, userModel);
            if (userModel == "") userModel = defaultModel;

            newAccount = {
                {"type", type},
                {"name", accountName},
                {"api_key", apiKey},
                {"api_url", defaultUrl},
                {"model", userModel}
            };

            accounts["accounts"].push_back(newAccount);
            saveAccounts("../accounts.json", accounts);
        }
        
    }

    if (newAccount.empty()) {
        std::cout << "Unknown provider. Try again.\n";
    }
    
    

}