#include "../include/selectaccount.h"
#include "../include/provider.h"
#include "../include/openai-compatible.h"
#include "../include/anthropic.h"
#include "../include/json.hpp"
#include <iostream>

using json = nlohmann::json;

std::unique_ptr<Provider> selectAccount(const json& accounts, const json& config) {
    std::string providerName;
    std::unique_ptr<Provider> provider = nullptr;

    while (true) {
        std::cout << "Pick a provider from the list below (/exit to leave):" << "\n";

        for (const auto& provider : accounts["accounts"]) {
            std::cout << "-- " << provider["name"].get<std::string>() << "\n";
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
                ); }

                else if (itprovider["type"].get<std::string>() == "openai-compatible") {
                    provider = std::make_unique<OpenAICompatible>(
                    itprovider["api_key"].get<std::string>(),
                    itprovider["api_url"].get<std::string>(),
                    itprovider["model"].get<std::string>(),
                    config["system_prompt"].get<std::string>(),
                    config["limit"].get<size_t>()
                ); }
                break;
            }
        }
        if (provider == nullptr) {
            std::cout << "Unexpected input. Try again.\n";
            continue;
        } else break;
    }
    return provider;
}