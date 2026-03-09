#pragma once
#include "provider.h"
#include <string>

class Anthropic : public Provider {
public:
    Anthropic(const std::string& api_key, const std::string& api_url,
            const std::string& model, const std::string& system_prompt, size_t limit, size_t max_tokens)
        : Provider(api_key, api_url, model, system_prompt, limit, max_tokens) {}
    
    Message send_request(const std::vector<Message>& conversation) const override;

    std::optional<std::string> extract_delta(const nlohmann::json& json) const override;
    // std::optional<ToolInfo> extract_tool_call(const nlohmann::json& json) const override;
};

