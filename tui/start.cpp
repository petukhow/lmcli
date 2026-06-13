/*
thread invariants:
    1. every single editing of data in working thread should be handled using screen.Post()
    2. every waitings (future.get()) are only allowed in working thread (if UI thread sleeps = bad UX)
    3. always join (t.join) thread after Loop
    4. only one working thread. busy flag is always initialized before thread
    5. active_promise is only valid when pending_command not empty (set_value only with this check) 
*/

#include "commands.h"
#include "json.hpp"
#include "providers/provider.h"
#include "select_account.h"
#include "constants.h"
#include "chats.h"
#include "loaders/config.h"
#include "loaders/accounts.h"
#include "loaders/json_io.h"
#include "types/message.h"
#include <cstddef>
#include <cstdio>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "types/roles.h"
#include "tools/handle_tool_calls.h"
#include "logging/logger.h"

#include "ftxui/component/component.hpp"         
#include "ftxui/component/component_base.hpp"
#include <ftxui/component/screen_interactive.hpp>
#include "ftxui/component/event.hpp"
#include <ftxui/component/animation.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>

#include <thread>
#include <atomic>
#include <future>
#include <pthread.h>

using json = nlohmann::json;
using namespace ftxui;

struct ChatValues {
    std::vector<Message> conversation;
    std::unique_ptr<Provider> account;
    std::string chats_path;
    size_t limit;
};

static std::optional<ChatValues> chat_init() {
    std::vector<Message> conversation;
    const auto config = load_config(CONFIG_FILE);
    const auto accounts = load_accounts(ACCOUNTS_FILE);

    if (config.is_null() || accounts.is_null()) {
        log(LogLevel::Error, "Failed to load config or accounts");
        return std::nullopt;
    }

    auto account = select_account(accounts, config);
    if (!account) {
        return std::nullopt;
    }

    const std::string chats_path = setup_chat();
    if (chats_path.empty()) return std::nullopt;

    const auto chats = load_json(chats_path);

    if (!chats.has_value()) {
        return std::nullopt;
    }

    if (chats->contains("conversation") && (*chats)["conversation"].is_array()) {
        conversation = (*chats)["conversation"].get<std::vector<Message>>();
    }

    if (!conversation.empty()) {
        if (conversation[0].content != config["system_prompt"].get<std::string>()) {
            conversation[0].content = config["system_prompt"];
        }
    }

    if (conversation.empty()) {
        conversation.push_back({Role::System, config["system_prompt"].get<std::string>(), "", {}});
    }

    return ChatValues{std::move(conversation), std::move(account), std::move(chats_path), config["limit"]};
}

void start() {
    Message prompt;
    Message output; 
    std::string streaming_buffer;
    std::string pending_command;
    std::atomic<bool> busy = false;
    std::promise<bool>* active_promise = nullptr;
    std::thread t;

    auto values = chat_init(); 
    if (!values) {
        log(LogLevel::Error, "Chat values not initialized");
        return;
    } 

    auto screen = ScreenInteractive::Fullscreen();
    auto input_prompt = Input(&prompt.content, " Write something...");
    auto component = Container::Horizontal({
        input_prompt
    });

    auto final_component = component | CatchEvent([&](Event event) {
        if (!pending_command.empty()) {
            if (event == Event::Character("y")) {
                active_promise->set_value(true);
                pending_command.clear();
                screen.RequestAnimationFrame();
                return true;
            }
            if (event == Event::Character("n")) {
                active_promise->set_value(false);
                pending_command.clear();
                screen.RequestAnimationFrame();
                return true;
            }
            return true;
        }

        if (event == Event::Return) {
            if (busy) return true;
            if (t.joinable()) t.join();
            
            auto trimmed = prompt.content;
            trimmed.erase(trimmed.find_last_not_of(" \n\r\t") + 1);
            if (trimmed.empty()) {
                prompt.content.clear();
                return true;
            }
            prompt.content = trimmed;

            if (prompt.content.empty()) return false;
            if (prompt.content == "/exit") {
                screen.Exit();
                return true;
            }
            values->conversation.push_back({Role::User, prompt.content,
                "", {}});
            log(LogLevel::Info, "User prompted: " + prompt.content);
            prompt.content.clear();

            busy = true;
            t = std::thread([&]{
                try {
                    auto output = values->account->send_request(values->conversation,
                        [&](const std::string& delta) {
                        screen.Post([&, delta] {
                            streaming_buffer += delta;
                            screen.RequestAnimationFrame();
                        });
                    });
                    while (!output.tool_calls.empty()) {
                        screen.Post([&] {
                            values->conversation.push_back({Role::Assistant, "", 
                                "", output.tool_calls});
                        });
                        auto results = handle_tool_calls(output, [&](const std::string& cmd) {
                            std::promise<bool> promise;
                            auto future = promise.get_future();
                            screen.Post([&, cmd] {
                                pending_command = cmd;
                                active_promise = &promise;
                                screen.RequestAnimationFrame();
                            });
                            return future.get();
                        });

                        std::promise<void> posted;
                        auto posted_future = posted.get_future();
                        screen.Post([&, results] {
                            for (const auto& msg : results) {
                                values->conversation.push_back(msg);  
                            }
                            posted.set_value();
                        });
                        posted_future.get();

                        log(LogLevel::Debug, "Number of tool calls: "
                            + std::to_string(output.tool_calls.size()));
                        output.tool_calls.clear();
                        output = values->account->send_request(values->conversation,
                            [&](const std::string& delta) {
                            screen.Post([&, delta] {
                                streaming_buffer += delta;
                                screen.RequestAnimationFrame();
                            });
                        });

                        const std::string is_failed = output.is_failed ? "true" : "false";
                        log(LogLevel::Debug, "API returned output: " + output.content);
                        log(LogLevel::Debug, "Is API request failed: " + is_failed);
                        log(LogLevel::Debug, "Number of tool calls: "
                            + std::to_string(output.tool_calls.size()));
                    }
                    
                    if (output.is_failed) {
                        log(LogLevel::Error, "Request failed with error: " + output.content);
                        screen.Post([&] {
                            values->conversation.pop_back();
                        });
                    }
                    else {
                        screen.Post([&, output] {
                            values->conversation.push_back({Role::Assistant, output.content,
                                "", {}});
                            streaming_buffer.clear();
                        });
                    }
                    log(LogLevel::Debug, "Model's output (after tool call): " + output.content);

                    if (values->limit > 0) {
                        screen.Post([&] {
                            while (values->conversation.size() > values->limit) {    
                                values->conversation.erase(values->conversation.begin() + 1);
                            }
                        });
                    }
                    std::promise<void> finished;
                    std::future<void> finished_ftr = finished.get_future();
                    screen.Post([&] { finished.set_value(); });
                    finished_ftr.get();
                    busy = false;
                } catch (const std::exception& e) {
                    log(LogLevel::Error, "Thread crashed: " + std::string(e.what()));
                    busy = false;
                }}
            );
            return true;
        }
        return false;
    });
    auto renderer = Renderer(final_component, [&] {
        Elements messages;
        for (const auto& msg : values->conversation) {
            if (msg.role == Role::User) {
                messages.push_back(paragraph("You: " + msg.content));
            } else if (msg.role == Role::Assistant) {
                messages.push_back(paragraph("Model: " + msg.content));
            }
        }

        if (!streaming_buffer.empty()) {
            messages.push_back(paragraph("Model: " + streaming_buffer));
        }

        if (!pending_command.empty()) {
            messages.push_back(text("Allow: " + pending_command + "? (y/n)") | color(Color::Yellow));
        }

        return vbox({
            vbox(messages) | flex,
            separator(),
            hbox(text("> "), input_prompt->Render()),
        }) | border;
    });

    screen.Loop(renderer);
    
    if (!pending_command.empty()) {
        active_promise->set_value(false);
    }
    if (t.joinable()) t.join();
    save_chat(values->chats_path, values->conversation);
}
