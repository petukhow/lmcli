#include "commands/select_account.h"
#include "terminal.h"
#include "providers/provider.h"
#include "json.hpp"
#include <iostream>
#include "logging/logger.h"
#include "utils/utils.h"

using json = nlohmann::json;

std::unique_ptr<Provider> select_account(const json& accounts, const json& config) {
    std::unique_ptr<Provider> provider;
    // Check if accounts array exists and is not empty
    if (accounts.contains("accounts") && !accounts["accounts"].empty()) {
        if (accounts["accounts"].size() == 1) {
            provider = Provider::create(accounts["accounts"][0], config);
            std::cout << "Automatically selected the only available account: " 
                << accounts["accounts"][0]["name"].get<std::string>() << "\n";
                log(LogLevel::Info, "Automatically selected the only available account: " + accounts["accounts"][0]["name"].get<std::string>());
        } else {
            while (true) {
                std::cout << "Select an account from the list below (type '/exit' to quit):\n";

                for (const auto& account : accounts["accounts"]) {
                    std::cout << "-- " << account["name"].get<std::string>() << "\n";
                }   

                const auto account = readline("> ");
                if (!account) break;

                clear_lines(accounts["accounts"].size() + 2);
                std::cout.flush();

                const std::string provider_name = *account;
                if (provider_name == "/exit") break;
                log(LogLevel::Info, "Account selected: " + provider_name);
                
                for (const auto& acc : accounts["accounts"]) {
                    if (acc["name"].get<std::string>() == provider_name) {
                        provider = Provider::create(acc, config);
                        break;
                    }
                }
                
                if (provider == nullptr) {
                    continue;
                } else {
                    break;
                    }
            }
        }
    } else {
        std::cerr << "Broken accounts.json config. Try 'lmcli setup' or 'lmcli init'";
        log(LogLevel::Error, "Broken accounts.json config");
    }
    return provider;
}