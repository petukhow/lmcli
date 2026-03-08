#include "commands.h"
#include "accounts.h"
#include "constants.h"
#include "providers.h"
#include "json.hpp"
#include <iostream>

using json = nlohmann::json;

void setup() {
    json providers = load_providers();
    json accounts = load_accounts(ACCOUNTS_FILE);

    std::string provider_name; // user's choice
    json new_account; // user's new account

    // config settings
    std::string default_model;
    std::string default_url;
    std::string type;

    // user's data
    std::string account_name;
    std::string api_key;
    std::string user_model;

    std::cout << "Select a provider to set up (type '/exit' to quit):\n";

    for (const auto& provider : providers["providers"]) {
        std::cout << "-- " << provider["name"].get<std::string>() << "\n";
    }

    std::cout << "> ";
    std::getline(std::cin, provider_name);

    if (provider_name == "/exit") {
        return;
    }

    for (const auto& provider : providers["providers"]) {
        if (provider["name"].get<std::string>() == provider_name) {
            type = provider["type"].get<std::string>();
            default_url = provider["default_api_url"].get<std::string>();
            default_model = provider["default_model"].get<std::string>();

            while (true) {
                std::cout << "Enter account name (Default: "
                    << provider["name"].get<std::string>() << "): ";
                std::getline(std::cin, account_name);

                bool duplicate = false;
                for (const auto& acc : accounts["accounts"]) {
                    if (acc["name"].get<std::string>() == account_name) {
                        std::cerr << "Error: Account with this name already exists.\n";
                        duplicate = true;
                        break;
                    }
                }
                if (duplicate) continue;

                if (account_name == "") account_name = provider["name"].get<std::string>();

                while (true) {
                    std::cout << "Enter API key: ";
                    std::getline(std::cin, api_key);
                    if (api_key.empty()) {
                        std::cerr << "Error: API key cannot be empty.\n";
                        continue;
                    }
                    break;
                }

                std::cout << "Enter model (Default: " << default_model << "): ";
                std::getline(std::cin, user_model);
                if (user_model == "") user_model = default_model;

                break;
            }
                new_account = {
                    {"type", type},
                    {"name", account_name},
                    {"api_key", api_key},
                    {"api_url", default_url},
                    {"model", user_model}
                };

                accounts["accounts"].push_back(new_account);
                save_accounts(accounts);
                break;
            }
    }

    if (new_account.empty()) {
        std::cout << "Unknown provider. Try again.\n";
    }
}