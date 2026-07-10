#include "render.h"
#include "chat_flow.h"

#include "ftxui/dom/elements.hpp"
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

using namespace ftxui;

Element render_hints(const std::vector<Hint>& hints) {
    Elements parts;
    for (size_t i = 0; i < hints.size(); ++i) {
        if (i > 0) parts.push_back(text(" · ") | color(Color::GrayDark));
        parts.push_back(text(hints[i].key) | bold | color(hints[i].key_color));
        parts.push_back(text(" " + hints[i].label) | color(Color::GrayDark));
    }
    return hbox(std::move(parts));
}

Element render_menu(const std::unique_ptr<ChatSession>& cs) {
    Elements lines;
    for (size_t i = 0; i < cs->menu_settings.menu_items.size(); ++i) {
        bool confirming = cs->menu_settings.delete_confirm_index.has_value()
            && *cs->menu_settings.delete_confirm_index == i;
        std::string label = cs->menu_settings.menu_items[i];
        if (confirming) label += "  (press 'd' again to delete)";

        auto line = text(label);
        if (confirming) {
            line = hbox(text("› "), line) | color(Color::Red);
        } else if (cs->menu_settings.menu_cursor == i) {
            line = hbox(text("› "), line) | color(cs->theme.prompt_color);
        }
        else {
            line = hbox(text("  "), line) | dim;
        }
        lines.push_back(line);
    }
    const std::vector<Hint> delete_hint = {{"d", "delete", cs->theme.status_color}};
    return vbox({
        text(cs->menu_settings.title),
        vbox(lines),
        cs->menu_settings.on_delete ? render_hints(delete_hint) : emptyElement(),
        text("")
    });
}

bool handle_scroll(Event event, float& scroll_pos) {
    if ((event.is_mouse() && event.mouse().button == Mouse::WheelUp)
        || event == Event::ArrowUp
        || event == Event::PageUp
    ) {
        scroll_pos -= 0.05f;
        scroll_pos = std::clamp(scroll_pos, 0.0f, 1.0f);
        return true;
    }
    if ((event.is_mouse() && event.mouse().button == Mouse::WheelDown)
        || event == Event::ArrowDown
        || event == Event::PageDown
    ) {
        scroll_pos += 0.05f;
        scroll_pos = std::clamp(scroll_pos, 0.0f, 1.0f);
        return true;
    }
    return false;
}