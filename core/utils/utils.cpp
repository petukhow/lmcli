#include "utils.h"
#include "logging/logger.h"
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <optional>
#include "linenoise.h"
#include "json.hpp"
#include "loaders/json_io.h"

std::optional<std::string> readline(const std::string& prompt) {
    char* raw = linenoise(prompt.c_str());
    if (!raw) return std::nullopt;
    const std::string result(raw);
    free(raw);
    return result;
}

std::string get_config_path(const std::string& filename) {
    const char* home = std::getenv("HOME");
    if (!home) {
        log(LogLevel::Info, "Fallback reached in get_config_path!");
        return "./" + filename;  // Fallback to current directory
    }
    return std::string(home) + "/.config/lmcli/" + filename;
}

std::string get_config_dir() {
    const char* home = std::getenv("HOME");
    if (!home) {
        log(LogLevel::Info, "Fallback reached in get_config_dir!");
        return "./"; // Fallback to current directory
    }
    return std::string(home) + "/.config/lmcli/";
}

std::string get_log_path(const std::string& filename) {
    const char* state_home = std::getenv("XDG_STATE_HOME");
    if (!state_home || std::string(state_home).empty()) {
        const char* home = std::getenv("HOME");
        if (!home) {
            log(LogLevel::Info, "Fallback reached in get_log_path!");
            return "./" + filename;
        }
        return std::string(home) + "/.local/state/lmcli/" + filename;
    }
    return std::string(state_home) + "/lmcli/" + filename;
}

std::string get_log_dir() {
    const char* state_home = std::getenv("XDG_STATE_HOME");
    if (!state_home || std::string(state_home).empty()) {
        const char* home = std::getenv("HOME");
        if (!home) {
            log(LogLevel::Info, "Fallback reached in get_log_dir!");
            return "./"; // Fallback to current directory
        }
        return std::string(home) + "/.local/state/lmcli/";
    }
    return std::string(state_home) + "/lmcli/";
}

std::string get_system_data_path(const std::string& filename) {
    const char* data_dirs = std::getenv("XDG_DATA_DIRS");
    std::string dirs_str = "/usr/local/share:/usr/share";
    if (data_dirs != nullptr && !std::string(data_dirs).empty()) {
        dirs_str = data_dirs;
    } 
    
    while (true) {
        const size_t colon = dirs_str.find(':');
        const std::string first_data_dir = (colon == std::string::npos) ? dirs_str : dirs_str.substr(0, colon);

        if (first_data_dir.empty()) return "/usr/share/lmcli/";

        const std::string base = first_data_dir.back() == '/' ? first_data_dir : first_data_dir + "/";
        const std::string cfg  = base + "lmcli/" + filename;
        if (std::filesystem::exists(cfg)) {
            return base + "lmcli/" + filename;
        } else {
            dirs_str.erase(0, first_data_dir.length() + (colon == std::string::npos ? 0 : 1));
            if (colon == std::string::npos) break;
        }
    }
    return "/usr/share/lmcli/" + filename;
}

void create_config_file_if_not_exists(const std::string& config_dir, const nlohmann::json& file_template) {
    if (std::filesystem::exists(config_dir)) {
        std::cout << "⚠ " << config_dir << " already exists, skipping\n";
        return;
    }
    // Create the file
    std::ofstream file(config_dir);
    if (!file.is_open()) {
        log(LogLevel::Error, "Could not create " + config_dir);
        std::cerr << "Error: Could not create " << config_dir << "\n";
        return;
    }

    file << file_template;
}

std::string create_file_if_not_exists(const std::string& chats_dir, const nlohmann::json& file_template) {
    size_t i = 1;
    std::filesystem::path final_path = chats_dir;
    const std::string stem = final_path.parent_path() / final_path.stem();
    const std::string extension = final_path.extension().string();

    while (std::filesystem::exists(final_path)) {
        final_path = stem + "_" + std::to_string(i) + extension;
        i++;
    }
    std::ofstream file(final_path);
    if (!file.is_open()) {
        log(LogLevel::Error, "Could not create " + std::string(final_path));
        std::cerr << "Error: Could not create " << final_path << "\n";
        return "";
    }
    file << file_template;
    return final_path.string();
}


std::string get_chats_dir() {
    const char* home = std::getenv("HOME");
    if (!home) {
        log(LogLevel::Info, "Fallback reached in get_chats_dir!");
        return "./";  // Fallback to current directory
    }
    return std::string(home) + "/.config/lmcli/chats/";
}

std::string get_chats_path(const std::string& filename) {
    const char* home = std::getenv("HOME");
    if (!home) {
        log(LogLevel::Info, "Fallback reached in get_chats_path!");
        return "./" + filename;  // Fallback to current directory
    }
    return std::string(home) + "/.config/lmcli/chats/" + filename;
}

nlohmann::json load_file_with_defaults(const std::string& filename, nlohmann::json defaults) {
    const std::optional<nlohmann::json> config = load_json(get_config_path(filename));
    if (!config.has_value()) return defaults;
    defaults.update(*config);
    return defaults;
}

nlohmann::json load_file_or_default(const std::string& filename, const nlohmann::json& defaults) {
    const auto data = load_json(get_config_path(filename));
    return data.has_value() ? *data : defaults;
}