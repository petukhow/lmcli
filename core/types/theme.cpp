#include "theme.h"
#include <cctype>
#include <cstdint>
#include <unordered_map>

static bool is_hex_color(const std::string& s) {
    if (s.size() != 7 || s[0] != '#') return false;
    for (size_t i = 1; i < 7; ++i) {
        if (!std::isxdigit(static_cast<unsigned char>(s[i]))) return false;
    }
    return true;
}

static uint8_t hex_byte(char hi, char lo) {
    auto nibble = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        return c - 'A' + 10;
    };
    return static_cast<uint8_t>(nibble(hi) * 16 + nibble(lo));
}

static ftxui::Color hex_to_color(const std::string& s) {
    return ftxui::Color::RGB(hex_byte(s[1], s[2]), hex_byte(s[3], s[4]), hex_byte(s[5], s[6]));
}

static const std::unordered_map<std::string, ftxui::Color> color_map = {
    {"black",         ftxui::Color::Black},
    {"red",           ftxui::Color::Red},
    {"green",         ftxui::Color::Green},
    {"yellow",        ftxui::Color::Yellow},
    {"blue",          ftxui::Color::Blue},
    {"magenta",       ftxui::Color::Magenta},
    {"cyan",          ftxui::Color::Cyan},
    {"white",         ftxui::Color::White},
    {"gray",          ftxui::Color::GrayDark},
    {"gray_light",    ftxui::Color::GrayLight},
    {"red_light",     ftxui::Color::RedLight},
    {"green_light",   ftxui::Color::GreenLight},
    {"yellow_light",  ftxui::Color::YellowLight},
    {"blue_light",    ftxui::Color::BlueLight},
    {"magenta_light", ftxui::Color::MagentaLight},
    {"cyan_light",    ftxui::Color::CyanLight},
};

ftxui::Color color_from_string(const std::string& s) {
    if (is_hex_color(s)) return hex_to_color(s);
    auto it = color_map.find(s);
    if (it != color_map.end()) return it->second;
    return ftxui::Color::White;
}

bool is_valid_color_name(const std::string& s) {
    return is_hex_color(s) || color_map.find(s) != color_map.end();
}

std::string valid_color_names() {
    std::string result;
    for (const auto& [name, color] : color_map) {
        (void)color;
        if (!result.empty()) result += ", ";
        result += name;
    }
    result += ", or a hex code (#rrggbb)";
    return result;
}

void from_json(const nlohmann::json& j, Theme& t) {
    t.name             = j.at("name").get<std::string>();
    t.user_color       = color_from_string(j.at("user_color").get<std::string>());
    t.assistant_color  = color_from_string(j.at("assistant_color").get<std::string>());
    t.status_color     = color_from_string(j.at("status_color").get<std::string>());
    t.border_color     = color_from_string(j.at("border_color").get<std::string>());
    t.prompt_color     = color_from_string(j.at("prompt_color").get<std::string>());
    t.streaming_color  = color_from_string(j.at("streaming_color").get<std::string>());
    t.separator_color  = color_from_string(j.at("separator_color").get<std::string>());
}
