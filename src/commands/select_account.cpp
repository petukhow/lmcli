#include "commands/select_account.h"
#include "utils/terminal.h"
#include "providers/provider.h"
#include "json.hpp"
#include <iostream>

using json = nlohmann::json;

std::unique_ptr<Provider> select_account(const json& accounts, const json& config) {
    std::string provider_name;
    std::unique_ptr<Provider> provider = nullptr;

    // Check if accounts array exists and is not empty
    if (accounts.contains("accounts") && !accounts["accounts"].empty()) {
        if (accounts["accounts"].size() == 1) {
            provider = Provider::create(accounts["accounts"][0], config);
            std::cout << "Automatically selected the only available account: " 
                      << accounts["accounts"][0]["name"].get<std::string>() << "\n";
        } else {
            while (true) {
                std::cout << "Select an account from the list below (type '/exit' to quit):\n";

                for (const auto& account : accounts["accounts"]) {
                    std::cout << "-- " << account["name"].get<std::string>() << "\n";
                }   

                
                std::cout << "> ";
                std::getline(std::cin, provider_name);

                clear_lines(accounts["accounts"].size() + 2);
                std::cout.flush();

                if (provider_name == "/exit") break;

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
        std::cerr << "Broken accounts.json config. Try 'lmcli init'.\n";
    }
    return provider;
}