#include "providers.h"

std::string_view provider_to_string(const Providers &p) {
    switch (p) {
        case Providers::Anthropic: return "anthropic";
        case Providers::OpenAICompatible: return "openai-compatible";
        case Providers::Google: return "google";
    }
    return "unknown";
}

Providers string_to_provider(const std::string& s) {
    if (s == "anthropic") return Providers::Anthropic;
    if (s == "openai-compatible") return Providers::OpenAICompatible;
    if (s == "google") return Providers::Google;
    return Providers::Anthropic;
}