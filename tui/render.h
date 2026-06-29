#pragma once
#include <functional>
#include <string>
#include <vector>

#include "ftxui/dom/elements.hpp"
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

struct ChatSession;

enum class Mode {
    Main,
    Menu,
    Form
};

struct FormSettings {
    std::function<bool()> on_submit;
};

struct MenuSettings {
    std::vector<std::string> menu_items;
    std::function<void(size_t)> on_select;
    size_t menu_cursor = 0;
    std::string title;
};

ftxui::Element render_menu(const std::unique_ptr<ChatSession>& cs);

bool handle_scroll(ftxui::Event event, float& scroll_pos);
