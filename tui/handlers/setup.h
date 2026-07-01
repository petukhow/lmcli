#pragma once
#include "chat_flow.h"
#include "json.hpp"
#include <ftxui/component/component_base.hpp>
#include <ftxui/dom/elements.hpp>

struct SetupFields {
    ftxui::Component form_name_input;
    ftxui::Component form_key_input;
    ftxui::Component form_model_input;
    ftxui::Component form_container;
};

void open_setup_menu(ChatSession& session);

ftxui::Element build_acc_errors_block(ChatSession& session, const nlohmann::json& accounts_list);

ftxui::Element render_setup_form(const ChatSession& session, const SetupFields& setup_fields, const ftxui::Element& acc_errors_block);