#include "providers.h"
#include "constants.h"
#include "json.hpp"
#include "loaders/json_io.h"
#include "utils/utils.h"

nlohmann::json load_providers() {
    return load_json(get_system_data_path(PROVIDERS_FILE));
}
