#include <curl/curl.h>
#include <memory>
#include <string>
#include "httpUtils.h"
#include "json.hpp"
#include "message.h"
#pragma once

class Provider {
protected:
    void performRequest(const std::string& body, const CurlSlist& headers, Curl& curl, std::string& rawResponse) const;

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
        
    virtual Message sendRequest(const std::vector<Message>& conversation) const = 0;
    virtual ~Provider() = default;
};

