#include "commands.h"
#include "accounts.h"
#include "json.hpp"
#include "config.h"
#include <iostream>

using json = nlohmann::json;

void setup() {
    json providers = loadProviders("providers.json");
    json accounts = loadAccounts("accounts.json");

    std::string providerName; // user's choice    
    json newAccount; // user's new account

    // config settings
    std::string defaultModel; 
    std::string defaultUrl;
    std::string type;

    // user's data
    std::string accountName;
    std::string apiKey;
    std::string userModel;

    std::cout << "Select a provider to set up (type '/exit' to quit):\n";

    for (const auto& provider : providers["providers"]) {
        std::cout << "-- " << provider["name"].get<std::string>() << "\n";
    }

    std::cout << "> ";
    std::getline(std::cin, providerName);

    if (providerName == "/exit") {
        return;
    }

    for (const auto& provider : providers["providers"]) {
        if (provider["name"].get<std::string>() == providerName) {
            type = provider["type"].get<std::string>();
            defaultUrl = provider["default_api_url"].get<std::string>();
            defaultModel = provider["default_model"].get<std::string>();

            std::cout << "Enter account name (If empty, it'll be named as a provider name): ";
            std::getline(std::cin, accountName);
            if (accountName == "") accountName = provider["name"].get<std::string>();
            
            std::cout << "Enter API key: ";
            std::getline(std::cin, apiKey);
            if (apiKey.empty()) {
                std::cerr << "API key cannot be empty.\n";
                continue;
            }

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
            saveAccounts("accounts.json", accounts);
            break;
        } 
    }

    if (newAccount.empty()) {
        std::cout << "Unknown provider. Try again.\n";
    }
}