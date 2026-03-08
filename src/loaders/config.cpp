#include "config.h"
#include "json.hpp"
#include "json_io.h"
#include "utils.h"
#include "constants.h"

nlohmann::json load_config(const std::string& filename) {
    const std::string full_path = get_config_path(filename);
    return load_json(full_path);
}

void save_config() {
    const std::string full_path = get_config_path(CONFIG_FILE);
    save_json(full_path);
}