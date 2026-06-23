#include "render.h"
#include "chat_flow.h"

#include "ftxui/dom/elements.hpp"
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

using namespace ftxui;

Element render_menu(const std::unique_ptr<ChatSession>& cs) {
    Elements lines;
    for (size_t i = 0; i < cs->menu_settings.menu_items.size(); ++i) {
        auto line = text(cs->menu_settings.menu_items[i]);
        if (cs->menu_settings.menu_cursor == i) {
            line = hbox(text("› "), line) | color(cs->theme.prompt_color);
        }
        else {
            line = hbox(text("  "), line) | dim;
        }
        lines.push_back(line);
    }
    return vbox({
        text(cs->menu_settings.title),
        vbox(lines),
        text("")
    });
}

void open_acc_menu(const std::unique_ptr<ChatSession>& cs, const nlohmann::json& accounts_list) {
    cs->menu_settings.menu_cursor = 0;
    cs->menu_settings.menu_items.clear();
    cs->menu_settings.title = "Select an account:";
    cs->prompt.content.clear();
    for (const auto& account : accounts_list) {
        cs->menu_settings.menu_items.push_back(account["name"].get<std::string>());
    }
}

void open_prov_menu(const std::unique_ptr<ChatSession>& cs, const nlohmann::json& providers) {
    cs->menu_settings.menu_cursor = 0;
    cs->menu_settings.menu_items.clear();
    cs->menu_settings.title = "Select a provider:";
    cs->prompt.content.clear();
    for (const auto& provider : providers) {
        cs->menu_settings.menu_items.push_back(provider["name"].get<std::string>());
    }
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