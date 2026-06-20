#include <optional>
#include <string>
#include "json.hpp"
#include "providers/provider.h"
#include "types/accounts.h"

bool name_exists(const nlohmann::json& accounts_list, std::string& acc_name) {
    for (const auto& acc : accounts_list) {
        if (acc["name"] == acc_name) return true;
    }
    return false;
}

static std::string make_unique_name(const nlohmann::json& accounts_list, const std::string& acc_name) {
    std::string unique = acc_name;
    size_t i = 0;
    while (name_exists(accounts_list, unique)) {
        unique = acc_name + '-' + std::to_string(++i); 
    }
    return unique;
}

std::optional<nlohmann::json> build_account(const AccountDraft& draft, const ProviderInfo& provider,
    const nlohmann::json& existing_accounts) {
    if (draft.api_key.empty()) return std::nullopt;

    std::string acc_name = draft.acc_name.empty() ? provider.default_name : draft.acc_name;
    std::string model = draft.model_name.empty() ? provider.default_model : draft.model_name;

    acc_name = make_unique_name(existing_accounts, acc_name);

    return nlohmann::json{
        {"type", provider.type},
        {"name", acc_name},
        {"api_key", draft.api_key},
        {"api_url", provider.default_url},
        {"model", model}
    };
}