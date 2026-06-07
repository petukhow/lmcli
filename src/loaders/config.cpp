#include "loaders/config.h"
#include "json.hpp"
#include "json_io.h"
#include "utils/utils.h"
#include "constants.h"
#include "logging/logger.h"

nlohmann::json load_config(const std::string& filename) {
    log(LogLevel::Info, "Loading config...");
    nlohmann::json defaults = {
        {"system_prompt", "You're a helpful assistant."},
        {"limit", 20},
        {"max_tokens", 1024},
        {"logging", false},
        {"blacklist", nlohmann::json::array({"reboot", "shutdown", "poweroff", "halt", "init 0", "init 6"})},
        {"confirm_required", nlohmann::json::array({"rm", "mv", "dd", "mkfs", "fdisk", "parted", "chmod 777", "chown"})}
    };
    // todo: remove repeated code at config.cpp and accounts.cpp
    std::optional<nlohmann::json> config = load_json(get_config_path(filename));
    if (!config.has_value()) return defaults;
    defaults.update(*config);
    
    return defaults;
}

void save_config(const nlohmann::json& config) {
    log(LogLevel::Info, "Saving config...");
    save_json(get_config_path(CONFIG_FILE), config);
}