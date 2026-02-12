#pragma once
#include "provider.h"

class OpenAICompatible : public Provider {
public:
    OpenAICompatible(const std::string& api_key, const std::string& api_url,
            const std::string& model, const std::string& system_prompt, size_t limit)
        : Provider(api_key, api_url, model, system_prompt, limit) {}
    
    Message sendRequest(const std::vector<Message>& conversation) const;
};