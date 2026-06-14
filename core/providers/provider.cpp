#include "logging/logger.h"
#include "utils/http_utils.h"
#include "open_ai_compatible.h"
#include "types/providers.h"
#include "google.h"
#include "anthropic.h"
#include "provider.h"
#include "json.hpp"
#include <curl/curl.h>
#include <functional>
#include <memory>
#include <iostream>
#include <optional>
#include <string>

std::unique_ptr<Provider> Provider::create(const nlohmann::json &accounts, const nlohmann::json &config) {
    std::unique_ptr<Provider> provider = nullptr;

    if (accounts["type"].get<std::string>() == Providers::OpenAICompatible.get_value()) {
        provider = std::make_unique<OpenAICompatible>(
        accounts["api_key"].get<std::string>(),
        accounts["api_url"].get<std::string>(),
        accounts["model"].get<std::string>(),
        config["system_prompt"].get<std::string>(),
         config["limit"].get<size_t>(),
        config["max_tokens"].get<size_t>()
        );
    }

    else if (accounts["type"].get<std::string>() == Providers::Anthropic.get_value()) {
        provider = std::make_unique<Anthropic>(
            accounts["api_key"].get<std::string>(),
            accounts["api_url"].get<std::string>(),
            accounts["model"].get<std::string>(),
            config["system_prompt"].get<std::string>(),
            config["limit"].get<size_t>(),
            config["max_tokens"].get<size_t>()
        );
    }
    else if (accounts["type"].get<std::string>() == Providers::Google.get_value()) {
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
        log(LogLevel::Error, "Unknown account type");
        return nullptr;
    }

    return provider;
}

StreamContext Provider::perform_request(const std::string& body, const CurlSlist& headers,
    Curl& curl, std::function<void(const std::string&)> callback,
    std::atomic<bool>* cancelled) const {
    StreamContext context;
    nlohmann::json parsed;
    context.provider = this;
    context.is_failed = false;
    context.cancelled = cancelled;
    context.callback = callback;
    
    CURLcode result;

    curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers.get());
    curl_easy_setopt(curl.get(), CURLOPT_URL, api_url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, stream_callback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &context);

    result = curl_easy_perform(curl.get());

    log(LogLevel::Debug, "buffer: " + context.buffer);
    log(LogLevel::Debug, "raw response: " + context.raw_response);

    long http_code = 0;
    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &http_code);
    log(LogLevel::Debug, "HTTP code: " + std::to_string(http_code));
    
    if (result != CURLE_OK) {
        context.is_failed = true;
    }

    log(LogLevel::Debug, "fallback check: full_content empty=" + std::to_string(context.full_content.empty()) + " raw empty=" + std::to_string(context.raw_response.empty()));

    if (context.full_content.empty() && !context.raw_response.empty()) {
        try {
            auto parsed = nlohmann::json::parse(context.raw_response);
            if (parsed.contains("error") && parsed["error"].contains("message")) {
                context.full_content = parsed["error"]["message"];
                context.is_failed = true;
            }
        } catch (const std::exception& e) {
            log(LogLevel::Error, "fallback parse failed: " + std::string(e.what()));
        }
    }

    return context;
}

void Provider::event_handler(StreamContext* context, std::function<void(const std::string&)> callback) const {
    std::string delim = event_delimiter();
    size_t event_end;
    while ((event_end = context->buffer.find(delim)) != std::string::npos) {
        std::optional<std::string> delta;
        std::string response;
        
        std::string event = context->buffer.substr(0, event_end);
        context->buffer.erase(0, event_end + delim.size());

        while (!event.empty()) {
            size_t data_index = event.find("data: ");
            if (data_index == std::string::npos) break;

            size_t subevent_end = event.find("\n", data_index);
            if (subevent_end == std::string::npos) {
                response = event.substr(data_index + 6);
            }
            else {
                response = event.substr(data_index + 6, subevent_end - data_index - 6);
            }

            if (response == "[DONE]") break;

            try {
                log(LogLevel::Debug, "Response: " + response);
                nlohmann::json parsed = nlohmann::json::parse(response);

                if (parsed.contains("type") && parsed["type"] == "message_stop") break;

                if (parsed.contains("error")) {
                    context->full_content = parsed["error"]["message"];
                    context->is_failed = true;
                    break;
                }

                delta = extract_delta(parsed);
                extract_tool_call(parsed, context);

            } catch (const std::exception& e) {
                log(LogLevel::Error, "Broken json.");
                std::cerr << "Broken response's json.\n";
            }

            if (delta.has_value()) {
                callback(*delta);
                context->full_content += *delta;
            }
            if (subevent_end == std::string::npos) event.clear();
            else event.erase(0, subevent_end + 1);
        }
        continue;
    }
}