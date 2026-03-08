#include "config.h"
#include "json.hpp"
#include "load_json.h"
#include "utils.h"

nlohmann::json load_config(const std::string& filepath) {
    const std::string fullpath = get_config_path(filepath);
    return load_json_file(fullpath);
}