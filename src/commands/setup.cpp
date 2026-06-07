#include "commands.h"
#include "loaders/accounts.h"
#include "constants.h"
#include "logging/logger.h"
#include "providers/providers.h"
#include "json.hpp"
#include "utils/utils.h"
#include <iostream>

using json = nlohmann::json;

void setup() {
    const json providers = load_providers();
    json accounts = load_accounts(ACCOUNTS_FILE);

    if (providers.empty()) {
        return;
    }

    std::string provider_name; // user's choice
    json new_account; // user's new account
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

    log(LogLevel::Info, "Provider selected.");

    for (const auto& provider : providers["providers"]) {
        if (provider["name"].get<std::string>() == provider_name) {
            const std::string type = provider["type"].get<std::string>();
            const std::string default_url = provider["default_api_url"].get<std::string>();
            const std::string default_model = provider["default_model"].get<std::string>();

            std::string account_name;
            while (true) {
                std::cout << "Enter account name (Default: "
                    << provider["name"].get<std::string>() << "): ";
                auto aname = readline();
                if (!aname) break;
                account_name = aname->empty() ? provider["name"].get<std::string>() : *aname;

                bool duplicate = false;
                for (const auto& acc : accounts["accounts"]) {
                    if (acc["name"].get<std::string>() == account_name) {
                        std::cerr << "Error: Account with given name already exists.\n";
                        log(LogLevel::Error, "Account with given name already exists.");
                        duplicate = true;
                        break;
                    }
                }
                if (duplicate) continue;

                while (true) {
                    std::cout << "Enter API key: ";
                    const auto akey = readline();
                    if (!akey || akey->empty()) {
                        std::cerr << "Error: API key cannot be empty.\n";
                        log(LogLevel::Error, "Invalid API key entered.");
                        continue;
                    }
                    else {
                        api_key = *akey;
                    }
                    break;
                }

                std::cout << "Enter model (Default: " << default_model << "): ";
                auto umodel = readline();
                if (!umodel) break;
                user_model = umodel->empty() ? default_model : *umodel;
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
                log(LogLevel::Info, "New account created");
                break;
            }
    }

    if (new_account.empty()) {
        std::cout << "Unknown provider. Try again.\n";
        log(LogLevel::Error, "Unknown provider");
    }
}