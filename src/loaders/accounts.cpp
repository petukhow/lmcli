#include "loaders/accounts.h"
#include "constants.h"
#include "json.hpp"
#include "json_io.h"
#include "logging/logger.h"
#include "utils/utils.h"

nlohmann::json load_accounts(const std::string& filename) {
    log(LogLevel::Info, "Loading accounts...");
    nlohmann::json defaults = ACCOUNTS_DEFAULT;
    // todo: remove repeated code at config.cpp and accounts.cpp
    const std::optional<nlohmann::json> accounts = load_json(get_config_path(filename));
    if (!accounts.has_value()) return defaults;
    else return accounts;
}

void save_accounts(const nlohmann::json& accounts) {
    log(LogLevel::Info, "Saving accounts...");
    save_json(get_config_path(ACCOUNTS_FILE), accounts);
}