#pragma once
#include "chat_flow.h"
#include <ftxui/component/component_base.hpp>
#include <ftxui/dom/elements.hpp>

struct ThemeFields {
    ftxui::Component name_input;
    ftxui::Component user_color_input;
    ftxui::Component assistant_color_input;
    ftxui::Component status_color_input;
    ftxui::Component border_color_input;
    ftxui::Component prompt_color_input;
    ftxui::Component streaming_color_input;
    ftxui::Component separator_color_input;
    ftxui::Component theme_container;
};

void open_theme_menu(ChatSession& session);

ftxui::Element build_theme_errors_block(const ChatSession& session);

ftxui::Element render_theme_form(const ChatSession& session, const ThemeFields& theme_fields, const ftxui::Element& theme_errors_block);
