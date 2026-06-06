#include "providers.h"
#include "constants.h"
#include "json.hpp"
#include "loaders/json_io.h"
#include "logging/logger.h"
#include "utils/utils.h"

nlohmann::json load_providers() {
    log(LogLevel::Info, "Loading providers...");
    return load_json(get_system_data_path(PROVIDERS_FILE));
}
