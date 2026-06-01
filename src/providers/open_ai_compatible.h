#pragma once
#include "json.hpp"
#include "provider.h"
#include "streaming.h"
#include <cstddef>
#include <string>
#include <iostream>

class OpenAICompatible : public Provider {
public:
    OpenAICompatible(const std::string& api_key, const std::string& api_url,
            const std::string& model, const std::string& system_prompt, size_t limit, size_t max_tokens)
        : Provider(api_key, api_url, model, system_prompt, limit, max_tokens) {
            std::cerr << "api_key: " << api_key << "\n";
        }
        
    Message send_request(const std::vector<Message>& conversation) const override;

    std::optional<std::string> extract_delta(const nlohmann::json& json) const override;
    void extract_tool_call(const nlohmann::json& json, StreamContext* context) const override;
};