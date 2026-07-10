#pragma once
#include <functional>
#include <optional>
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
    std::function<void(size_t)> on_delete;
    size_t menu_cursor = 0;
    std::string title;
    std::optional<size_t> delete_confirm_index;
};

ftxui::Element render_menu(const std::unique_ptr<ChatSession>& cs);

bool handle_scroll(ftxui::Event event, float& scroll_pos);

// A single "key label" keybinding hint, meant to be rendered in a row via
// render_hints(). Adding a new keybinding hint anywhere is just one more
// entry in a hints vector -- no ad-hoc footer string wrangling.
struct Hint {
    std::string key;
    std::string label;
    ftxui::Color key_color = ftxui::Color::GrayDark;
};

ftxui::Element render_hints(const std::vector<Hint>& hints);
