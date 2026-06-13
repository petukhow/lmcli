#include "theme.h"
#include <unordered_map>

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
    auto it = color_map.find(s);
    if (it != color_map.end()) return it->second;
    return ftxui::Color::White;
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
