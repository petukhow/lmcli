#pragma once
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <future>
#include <thread>
#include "types/message.h"
#include "types/theme.h"
#include "providers/provider.h"

struct ChatSession {
    std::vector<Message> conversation;
    std::unique_ptr<Provider> account;
    std::string chats_path;
    size_t limit;
    Theme theme;

    Message prompt;
    std::string error_message;
    std::string streaming_buffer;
    std::string pending_command;
    int scroll_up = 0;
    std::atomic<bool> busy{false};
    std::atomic<bool> cancelled{false};
    std::promise<bool>* active_promise = nullptr;
    std::thread worker;
};

std::unique_ptr<ChatSession> chat_init();
