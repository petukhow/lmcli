#include "utils.h"
#include "logging/logger.h"
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>

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
        size_t colon = dirs_str.find(':');
        std::string first_data_dir = (colon == std::string::npos) ? dirs_str : dirs_str.substr(0, colon);

        if (first_data_dir.empty()) return "/usr/share/lmcli/";

        std::string base = first_data_dir.back() == '/' ? first_data_dir : first_data_dir + "/";
        std::string cfg  = base + "lmcli/" + filename;
        if (std::filesystem::exists(cfg)) {
            return base + "lmcli/" + filename;
        } else {
            dirs_str.erase(0, first_data_dir.length() + (colon == std::string::npos ? 0 : 1));
            if (colon == std::string::npos) break;
        }
    }
    return "/usr/share/lmcli/" + filename;
}

void create_config_file_if_not_exists(const std::string& config_dir, const std::string& file_template) {
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

std::string create_file_if_not_exists(const std::string& chats_dir, const std::string& file_template) {
    size_t i = 1;
    std::filesystem::path final_path = chats_dir;
    std::string stem = final_path.parent_path() / final_path.stem();
    std::string extension = final_path.extension().string();

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