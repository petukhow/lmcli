#include "openAICompatible.h"
#include "anthropic.h"
#include "provider.h"
#include "json.hpp"
#include <memory>
#include <iostream>

std::unique_ptr<Provider> Provider::create(const nlohmann::json &accounts, const nlohmann::json &config) {
    std::unique_ptr<Provider> provider = nullptr;

    if (accounts["type"].get<std::string>() == "openai-compatible") {
        provider = std::make_unique<OpenAICompatible>(
        accounts["api_key"].get<std::string>(),
        accounts["api_url"].get<std::string>(),
        accounts["model"].get<std::string>(),
        config["system_prompt"].get<std::string>(),
         config["limit"].get<size_t>(),
        config["max_tokens"].get<size_t>()
        );
    }

    else if (accounts["type"].get<std::string>() == "anthropic") {
        provider = std::make_unique<Anthropic>(
            accounts["api_key"].get<std::string>(),
            accounts["api_url"].get<std::string>(),
            accounts["model"].get<std::string>(),
            config["system_prompt"].get<std::string>(),
            config["limit"].get<size_t>(),
            config["max_tokens"].get<size_t>()
        );
    }
    else {
        std::cerr << "No accounts configured. Run 'lmcli setup' to add one.\n";
        return nullptr;
    }
    std::cerr << "provider is: " << (provider == nullptr ? "null" : "not null") << "\n";

    return provider;
}
