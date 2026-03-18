#include "providers.h"
#include "constants.h"
#include "json.hpp"
#include "loaders/json_io.h"
#include "utils/utils.h"

nlohmann::json load_providers() {
    std::string full_path = get_system_data_path(PROVIDERS_FILE);
    return load_json(full_path);
}
