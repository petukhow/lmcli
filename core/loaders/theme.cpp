#include "theme.h"
#include "json.hpp"
#include "json_io.h"
#include "utils/utils.h"
#include "logging/logger.h"
#include "constants.h"

static Theme default_theme() {
    return Theme{
        "tech",
        ftxui::Color::White,
        ftxui::Color::Cyan,
        ftxui::Color::Yellow,
        ftxui::Color::Blue,
        ftxui::Color::Green,
        ftxui::Color::CyanLight,
        ftxui::Color::GrayDark,
    };
}

Theme load_theme(const std::string& theme_name) {
    const auto data = load_json(get_config_path(THEMES_FILE));
    if (!data.has_value() || !data->contains("themes") || !(*data)["themes"].is_array()) {
        log(LogLevel::Info, "No themes file found, using default theme");
        return default_theme();
    }

    for (const auto& entry : (*data)["themes"]) {
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
