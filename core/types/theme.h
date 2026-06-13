#pragma once
#include "json.hpp"
#include <ftxui/dom/elements.hpp>
#include <string>

struct Theme {
    std::string name;
    ftxui::Color user_color;
    ftxui::Color assistant_color;
    ftxui::Color status_color;
    ftxui::Color border_color;
    ftxui::Color prompt_color;
    ftxui::Color streaming_color;
    ftxui::Color separator_color;
};

ftxui::Color color_from_string(const std::string& s);
void from_json(const nlohmann::json& j, Theme& t);
