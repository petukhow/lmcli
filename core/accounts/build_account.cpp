#include <optional>
#include "json.hpp"
#include "providers/provider.h"
#include "types/accounts.h"

static std::string make_unique_name(const nlohmann::json& accounts_list, const std::string& acc_name) {
    int i = 1;
    std::string base = acc_name;
    while (true) {
        bool exists = false;
        for (const auto& acc : accounts_list) {
            if (acc["name"].get<std::string>() == acc_name) {
                exists = true;
                break;
            }
        }
        if (!exists) return acc_name;
        return base + "-" + std::to_string(++i);
    }
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