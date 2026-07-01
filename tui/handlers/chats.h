#pragma once
#include "chat_flow.h"
#include <ftxui/component/component_base.hpp>
#include <ftxui/dom/elements.hpp>

void open_chat(ChatSession& cs, const std::string& path);

void close_chat(ChatSession& cs);

void open_chats_menu(ChatSession& session);

ftxui::Element render_chats_form(const ChatSession& session, const ftxui::Component& chat_name_input);