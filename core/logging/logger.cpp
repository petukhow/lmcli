#include "logger.h"
#include <string>
#include "utils/utils.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <filesystem>
#include "loaders/json_io.h"
#include "constants.h"

bool logging = false;

void log(LogLevel level, const std::string& event) {
    if (!logging) return;
    std::time_t now = std::time(nullptr);
    std::tm* local_time = std::localtime(&now);
    
    std::string level_str;
    switch (level) {
        case LogLevel::Debug: level_str = "DEBUG"; break;
        case LogLevel::Info:  level_str = "INFO";  break;
        case LogLevel::Error: level_str = "ERROR"; break;
    }

    std::ostringstream entry;
    entry << std::put_time(local_time, "%Y-%m-%d %H:%M:%S")
          << " [" << level_str << "] "
          << event << "\n";

    const std::string log_path = get_log_path(LOGS_FILE);
    const bool existed = std::filesystem::exists(log_path);

    std::ofstream file(log_path, std::ios::app);
    if (file.is_open()) {
        file << entry.str();
    }
    file.close();

    if (!existed) {
        std::filesystem::permissions(log_path,
            std::filesystem::perms::owner_read | std::filesystem::perms::owner_write,
            std::filesystem::perm_options::replace);
    }
}

// bool = "if logger initialized?"
bool logger_init() {
    // check if config loaded and logging settings are actual
    std::optional<nlohmann::json> check_config = load_json(get_config_path(CONFIG_FILE));
    if (!check_config.has_value()) {
        logging = true; // enable alarm mode
        return false; 
    }
    else {
        logging = (*check_config)["logging"].get<bool>(); 
        return true;
    }
}