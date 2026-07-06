#include "theme.h"
#include "json.hpp"
#include "json_io.h"
#include "utils/utils.h"
#include "logging/logger.h"
#include "constants.h"

static Theme default_theme() {
    return Theme{
        "tech",
        ftxui::Color::Cyan,
        ftxui::Color::White,
        ftxui::Color::Yellow,
        ftxui::Color::White,
        ftxui::Color::Cyan,
        ftxui::Color::White,
        ftxui::Color::GrayDark,
    };
}

nlohmann::json load_themes_file() {
    return load_file_with_defaults(THEMES_FILE, THEMES_DEFAULT);
}

void save_themes_file(const nlohmann::json& themes_data) {
    save_json(get_config_path(THEMES_FILE), themes_data);
}

bool theme_name_exists(const nlohmann::json& themes_data, const std::string& theme_name) {
    if (!themes_data.contains("themes") || !themes_data["themes"].is_array()) return false;
    for (const auto& entry : themes_data["themes"]) {
        if (entry.contains("name") && entry["name"] == theme_name) return true;
    }
    return false;
}

Theme load_theme(const std::string& theme_name) {
    const auto data = load_themes_file();
    if (!data.contains("themes") || !data["themes"].is_array()) {
        log(LogLevel::Info, "No themes file found, using default theme");
        return default_theme();
    }

    for (const auto& entry : data["themes"]) {
        if (entry.contains("name") && entry["name"] == theme_name) {
            try {
                return entry.get<Theme>();
            } catch (const std::exception& e) {
                log(LogLevel::Error, "Failed to parse theme '" + theme_name + "': " + e.what());
                return default_theme();
            }
        }
    }

    log(LogLevel::Info, "Theme '" + theme_name + "' not found, using default");
    return default_theme();
}
