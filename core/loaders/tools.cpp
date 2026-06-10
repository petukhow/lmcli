#include "json.hpp"
#include "tools.h"
#include "constants.h"
#include "logging/logger.h"
#include "utils/utils.h"

nlohmann::json load_tools(const std::string& filename) {
    log(LogLevel::Info, "Loading tools...");
    return load_file_with_defaults(filename, TOOLS_DEFAULT);
}