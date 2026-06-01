#include "json.hpp"
#include "tools.h"
#include "constants.h"
#include "utils/utils.h"
#include "json_io.h"

nlohmann::json load_tools() {
    return load_json(get_system_data_path(TOOLS_FILE));
}