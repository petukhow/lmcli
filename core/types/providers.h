#pragma once
#include <string_view>
#include <string>
#include "json.hpp"

enum class Providers {
    Anthropic, 
    OpenAICompatible,
    Google
};

std::string_view provider_to_string(const Providers& p);
Providers string_to_provider(const std::string& s);

inline void to_json(nlohmann::json& j, const Providers& p) {
    j = std::string(provider_to_string(p));
}

inline void from_json(const nlohmann::json& j, Providers& p) {
    p = string_to_provider(j.get<std::string>());
}