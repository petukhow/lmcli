#include "httpUtils.h"
#include "openAICompatible.h"
#include "google.h"
#include "streaming.h"
#include "anthropic.h"
#include "provider.h"
#include "json.hpp"
#include <memory>
#include <iostream>
#include <utility>

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
            config["limit"].get<size_t>(),
            config["max_tokens"].get<size_t>()
        );
    }
    else {
        std::cerr << "Unknown account type.\n";
        return nullptr;
    }

    return provider;
}

std::pair<std::string, bool> Provider::performRequest(const std::string& body, const CurlSlist& headers,
    Curl& curl) const {
    StreamContext context;
    context.provider = this;
    context.isFailed = false;
    
    CURLcode result;

    curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers.get());
    curl_easy_setopt(curl.get(), CURLOPT_URL, api_url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, streamCallback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &context);

    result = curl_easy_perform(curl.get());

    if (result != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed:\n";
        std::cerr << curl_easy_strerror(result) << "\n";
        context.isFailed = true;
    }

    return {context.fullContent, context.isFailed};
}
