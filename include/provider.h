#pragma once
#include <curl/curl.h>
#include <memory>
#include <optional>
#include <string>
#include "http_utils.h"
#include "json.hpp"
#include "message.h"
#include "streaming.h"
#include "tools.h"

class Provider {
protected:
    StreamContext perform_request(const std::string& body, const CurlSlist& headers, Curl& curl) const;

    std::string api_key;
    std::string api_url;
    std::string model;
    std::string system_prompt;
    size_t limit;
    size_t max_tokens;

public:
    Provider(const std::string& api_key, const std::string& api_url,
        const std::string& model, const std::string& system_prompt, size_t limit, size_t max_tokens)
        : api_key(api_key), api_url(api_url), model(model), system_prompt(system_prompt), limit(limit), max_tokens(max_tokens) {}

    static std::unique_ptr<Provider> create(const nlohmann::json& accounts, const nlohmann::json& config);
    void set_model(const std::string& new_model) { model = new_model; }
        
    virtual Message send_request(const std::vector<Message>& conversation) const = 0;
    virtual std::optional<std::string> extract_delta(const nlohmann::json& json) const = 0;
    virtual std::optional<ToolInfo> extract_tool_call(const nlohmann::json& json) const { return std::nullopt; };
    virtual void event_handler(StreamContext* context) const;
    virtual ~Provider() = default;
};

