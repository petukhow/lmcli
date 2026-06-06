#include "loaders/config.h"
#include "json.hpp"
#include "json_io.h"
#include "utils/utils.h"
#include "constants.h"
#include "logging/logger.h"

nlohmann::json load_config(const std::string& filename) {
    log(LogLevel::Info, "Loading config...");
    return load_json(get_config_path(filename));
}

void save_config(const nlohmann::json& config) {
    log(LogLevel::Info, "Saving config...");
    save_json(get_config_path(CONFIG_FILE), config);
}