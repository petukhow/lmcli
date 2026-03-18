#include "loaders/accounts.h"
#include "constants.h"
#include "json.hpp"
#include "json_io.h"
#include "utils/utils.h"

nlohmann::json load_accounts(const std::string& filename) {
    const std::string full_path = get_config_path(filename);
    return load_json(full_path);
}

void save_accounts(const nlohmann::json& accounts) {
    std::string full_path = get_config_path(ACCOUNTS_FILE);
    save_json(full_path, accounts);
}