#include "loaders/accounts.h"
#include "constants.h"
#include "json.hpp"
#include "json_io.h"
#include "utils/utils.h"

nlohmann::json load_accounts(const std::string& filename) {
    return load_json(get_config_path(filename));
}

void save_accounts(const nlohmann::json& accounts) {
    save_json(get_config_path(ACCOUNTS_FILE), accounts);
}