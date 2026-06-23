#pragma once
#include <string>
#include <vector>
#include <optional>

struct ConfigDraft {
    std::string system_prompt;
    std::string limit;
    std::string max_tokens;
    bool logging;

    std::vector<std::string> confirm_required;
    std::vector<std::string> restricted;
    std::string confirm_required_raw;
    std::string restricted_raw;
    
    std::optional<std::string> limit_error = std::nullopt;
    std::optional<std::string> max_tokens_error = std::nullopt;
    std::optional<std::string> confirm_required_error = std::nullopt;
    std::optional<std::string> restricted_error = std::nullopt;
};