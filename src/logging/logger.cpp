#include "logger.h"
#include <string>
#include "loaders/config.h"
#include "utils/utils.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>
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

    std::ofstream file(get_log_path(LOGS_FILE), std::ios::app);
    if (file.is_open()) {
        file << entry.str();
    }
}

void logger_init() {
    nlohmann::json config = load_config(CONFIG_FILE);
    logging = config["logging"].get<bool>();
}