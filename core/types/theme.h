#pragma once
#include "json.hpp"
#include <ftxui/dom/elements.hpp>
#include <optional>
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

struct ThemeDraft {
    std::string name;
    std::string user_color;
    std::string assistant_color;
    std::string status_color;
    std::string border_color;
    std::string prompt_color;
    std::string streaming_color;
    std::string separator_color;

    std::optional<std::string> name_error;
    std::optional<std::string> color_error;
};

ftxui::Color color_from_string(const std::string& s);
bool is_valid_color_name(const std::string& s);
std::string valid_color_names();
void from_json(const nlohmann::json& j, Theme& t);
