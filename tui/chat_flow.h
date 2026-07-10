#pragma once
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <future>
#include <thread>
#include "types/accounts.h"
#include "types/config.h"
#include "types/message.h"
#include "types/theme.h"
#include "providers/provider.h"
#include "render.h"

struct ChatSession {
    std::vector<Message> conversation;
    std::unique_ptr<Provider> account;
    std::string account_name;
    std::optional<std::string> chats_path;
    size_t limit;
    Theme theme;

    Message prompt;
    std::string active_form; // form to render from command (setup, config)
    std::string startup_error;
    std::string error_message;
    std::string streaming_buffer;
    std::string pending_command;
    float scroll_pos = 1.0f;
    int active_tab;
    bool exit_confirm_pending = false;
    bool show_tool_output = false;
    std::atomic<bool> busy{false};
    std::atomic<bool> cancelled{false};
    std::promise<bool>* active_promise = nullptr;
    std::thread worker;

    Mode mode = Mode::Main;
    FormSettings form;
    AccountDraft account_draft;
    MenuSettings menu_settings;
    ConfigDraft config_draft;
    ThemeDraft theme_draft;
    std::string chat_name_input;
};
    
std::unique_ptr<ChatSession> chat_init();
