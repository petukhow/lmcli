#pragma once
#include "chat_flow.h"
#include <ftxui/component/component_base.hpp>
#include <ftxui/dom/elements.hpp>

struct ConfigFields {
    ftxui::Component config_prompt_input;
    ftxui::Component config_limit_input;
    ftxui::Component config_tokens_input;
    ftxui::Component config_logging_toggle;
    ftxui::Component config_confirm_input;
    ftxui::Component config_restricted_input;
    ftxui::Component config_container;
};

void open_config_menu(ChatSession& session);

ftxui::Element build_conf_errors_block(const ChatSession& session);

ftxui::Element render_config_form(const ChatSession& session, const ConfigFields& config_fields, const ftxui::Element& conf_errors_block);