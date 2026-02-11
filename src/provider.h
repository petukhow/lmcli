#include <iostream>
#include <vector>
#include "message.h"

class Provider {
private:
    std::string api_key;
    std::string api_url;
    std::string model;
    std::string system_prompt;
    size_t limit;

public:
    Provider(const std::string& api_key, const std::string& api_url, const std::string& model, const std::string& system_prompt, size_t limit)
        : api_key(api_key), api_url(api_url), model(model), system_prompt(system_prompt), limit(limit) {}
    virtual Message sendRequest(const std::vector<Message>& conversation) const = 0;
    virtual ~Provider() = default;
};