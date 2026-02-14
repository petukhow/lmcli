#include "selectAccount.h"
#include "provider.h"
#include "openAICompatible.h"
#include "anthropic.h"
#include "json.hpp"
#include <iostream>

using json = nlohmann::json;

std::unique_ptr<Provider> selectAccount(const json& accounts, const json& config) {
    std::string providerName;
    std::unique_ptr<Provider> provider = nullptr;

    // Check if accounts array exists and is not empty
    if (accounts.contains("accounts") && !accounts["accounts"].empty()) {
        while (true) {
            std::cout << "Pick an account from the list below (/exit to leave):" << "\n";

            for (const auto& account : accounts["accounts"]) {
                std::cout << "-- " << account["name"].get<std::string>() << "\n";
            }   

            std::cout << "> ";
            std::getline(std::cin, providerName);
            if (providerName == "/exit") break;

            for (const auto& itprovider : accounts["accounts"]) {
                if (itprovider["name"].get<std::string>() == providerName) {
                    if (itprovider["type"].get<std::string>() == "anthropic") {
                        provider = std::make_unique<Anthropic>(
                            itprovider["api_key"].get<std::string>(),
                            itprovider["api_url"].get<std::string>(),
                            itprovider["model"].get<std::string>(),
                            config["system_prompt"].get<std::string>(),
                            config["limit"].get<size_t>()
                        );
                        break;
                    }
                    else if (itprovider["type"].get<std::string>() == "openai-compatible") {
                        provider = std::make_unique<OpenAICompatible>(
                            itprovider["api_key"].get<std::string>(),
                            itprovider["api_url"].get<std::string>(),
                            itprovider["model"].get<std::string>(),
                            config["system_prompt"].get<std::string>(),
                            config["limit"].get<size_t>()
                        );
                        break;
                    }
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
        std::cout << "Looks like you have no accounts registered. Register it with 'lmcli setup' command in terminal.\n";
    }
    
    return provider;
}