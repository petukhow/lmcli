#include "config.h"
#include "json.hpp"
#include "load_json.h"
#include "utils.h"

nlohmann::json load_config(const std::string& filename) {
    const std::string full_path = get_config_path(filename);
    return load_json_file(full_path);
}