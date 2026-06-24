#pragma once
#include "json.hpp"
#include <optional>
#include "providers/provider.h"
#include "types/accounts.h"

std::optional<nlohmann::json> build_account(const AccountDraft& draft, const ProviderInfo& provider, 
    const nlohmann::json& existing_accounts);

bool name_exists(const nlohmann::json& accounts_list, std::string& acc_name);