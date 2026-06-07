#include "providers.h"
#include "constants.h"
#include "json.hpp"
#include "loaders/json_io.h"
#include "logging/logger.h"
#include "utils/utils.h"

nlohmann::json load_providers(const std::string& filename) {
    log(LogLevel::Info, "Loading providers...");
    nlohmann::json defaults = PROVIDERS_DEFAULT;
    const std::optional<nlohmann::json> providers = load_json(get_config_path(filename));
    if (!providers.has_value()) return defaults;
    defaults.update(*providers);

    return defaults;
}
