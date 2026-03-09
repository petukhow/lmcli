#include "http_utils.h"
#include "open_ai_compatible.h"
#include "google.h"
#include "streaming.h"
#include "anthropic.h"
#include "provider.h"
#include "json.hpp"
#include <memory>
#include <iostream>
#include <optional>
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

std::pair<std::string, bool> Provider::perform_request(const std::string& body, const CurlSlist& headers,
    Curl& curl) const {
    StreamContext context;
    nlohmann::json parsed;
    context.provider = this;
    context.is_failed = false;
    
    CURLcode result;

    curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers.get());
    curl_easy_setopt(curl.get(), CURLOPT_URL, api_url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, stream_callback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &context);

    

    result = curl_easy_perform(curl.get());

    if (result != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed:\n";
        std::cerr << curl_easy_strerror(result) << "\n";
        context.is_failed = true;
    }

    if (context.full_content.empty() && !context.buffer.empty()) {
    try {
        auto parsed = nlohmann::json::parse(context.buffer);
        if (parsed.contains("error")) {
            context.full_content = parsed["error"]["message"];
            context.is_failed = true;
        }
    } catch (...) {
        }
    }

    return {context.full_content, context.is_failed};
}

void Provider::event_handler(StreamContext* context) const {
    size_t event_end;
    while ((event_end = context->buffer.find("\n\n")) != std::string::npos) {
        std::optional<std::string> delta;
        std::string response;

        std::string event = context->buffer.substr(0, event_end);
        context->buffer.erase(0, event_end + 2);

        size_t data_index = event.find("data: ");
        if (data_index == std::string::npos) continue;
        response = event.substr(data_index + 6);

        if (response == "[DONE]") break;

        try {
            nlohmann::json parsed = nlohmann::json::parse(response);

            if (parsed.contains("type") && parsed["type"] == "message_stop") break;
            if (parsed.contains("candidates") && parsed["candidates"][0].contains("finishReason")) break;

            if (parsed.contains("error")) {
                context->full_content = parsed["error"]["message"];
                context->is_failed = true;
                break;
            }

            delta = extract_delta(parsed);
        } catch (const std::exception& e) {
            std::cerr << "Broken response's json.\n";
        }

        if (delta.has_value()) {
            std::cout << *delta;
            std::cout.flush();
            context->full_content += *delta;
        }
    }
}
