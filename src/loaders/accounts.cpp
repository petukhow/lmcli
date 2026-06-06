#include "loaders/accounts.h"
#include "constants.h"
#include "json.hpp"
#include "json_io.h"
#include "logging/logger.h"
#include "utils/utils.h"

nlohmann::json load_accounts(const std::string& filename) {
    log(LogLevel::Info, "Loading accounts...");
    return load_json(get_config_path(filename));
}

void save_accounts(const nlohmann::json& accounts) {
    log(LogLevel::Info, "Saving accounts...");
    save_json(get_config_path(ACCOUNTS_FILE), accounts);
}