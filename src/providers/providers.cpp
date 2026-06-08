#include "providers.h"
#include "constants.h"
#include "json.hpp"
#include "logging/logger.h"
#include "utils/utils.h"

nlohmann::json load_providers(const std::string& filename) {
    log(LogLevel::Info, "Loading providers...");
    return load_file_with_defaults(filename, PROVIDERS_DEFAULT);
}
