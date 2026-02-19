#include "httpUtils.h"
#include "openAICompatible.h"
#include "google.h"
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
    else if (accounts["type"].get<std::string>() == "google") {
        provider = std::make_unique<Google>(
            accounts["api_key"].get<std::string>(),
            accounts["api_url"].get<std::string>(),
            accounts["model"].get<std::string>(),
            config["system_prompt"].get<std::string>(),
            config["limit"].get<size_t>()
        );
    }
    else {
        std::cerr << "No accounts configured. Run 'lmcli setup' to add one.\n";
        return nullptr;
    }

    return provider;
}

void Provider::performRequest(const std::string& body, const CurlSlist& headers,
    Curl& curl, std::string& rawResponse) const {
    CURLcode result;

    curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers.get());
    curl_easy_setopt(curl.get(), CURLOPT_URL, api_url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &rawResponse);

    result = curl_easy_perform(curl.get());

    if (result != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed:\n";
        std::cerr << curl_easy_strerror(result) << "\n";
    }
}
