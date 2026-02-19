#pragma once
#include "provider.h"
#include <cstddef>
#include <string>

class OpenAICompatible : public Provider {
private:
    size_t max_tokens;
public:
    OpenAICompatible(const std::string& api_key, const std::string& api_url,
            const std::string& model, const std::string& system_prompt, size_t limit, size_t max_tokens)
        : Provider(api_key, api_url, model, system_prompt, limit), max_tokens(max_tokens) {}
    
    Message sendRequest(const std::vector<Message>& conversation) const override;
};