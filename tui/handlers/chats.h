#pragma once
#include "chat_flow.h"

void open_chat(ChatSession& cs, const std::string& path);

void close_chat(ChatSession& cs);

void open_chats_menu(ChatSession& session);