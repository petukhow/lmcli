#include "json.hpp"
#include "tools.h"
#include "constants.h"
#include "logging/logger.h"
#include "utils/utils.h"
#include "json_io.h"

nlohmann::json load_tools(const std::string& filename) {
    log(LogLevel::Info, "Loading tools...");
    nlohmann::json defaults = TOOLS_DEFAULT;
    const std::optional<nlohmann::json> tools = load_json(get_config_path(filename));
    if (!tools.has_value()) return defaults;
    defaults.update(*tools);

    return defaults;
}