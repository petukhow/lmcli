#pragma once
#include <optional>
#include <string>

struct AccountDraft {
    std::string acc_name;
    std::string api_key;
    std::string model_name;

    std::optional<std::string> key_error;
    std::optional<std::string> name_error;
};