#include "providers.h"
#include "constants.h"
#include "json.hpp"
#include "load_json.h"
#include "utils.h"

nlohmann::json load_providers() {
    std::string full_path = get_system_data_path(PROVIDERS_FILE);
    return load_json_file(full_path);
}
