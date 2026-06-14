/*
thread invariants:
    1. every single editing of shared data should be handled using screen.Post() (UI thread only)
    2. every waitings (future.get()) are only allowed in working thread (if UI thread sleeps = bad UX)
    3. only one working thread. busy flag is always initialized before thread
    4. active_promise is only valid when pending_command not empty (set_value only with this check) 
*/

#include "chat_flow.h"
#include "chats.h"
#include "commands.h"
#include "tools/handle_tool_calls.h"
#include "logging/logger.h"
#include "types/roles.h"

#include "ftxui/component/component.hpp"
#include <ftxui/component/screen_interactive.hpp>
#include "ftxui/component/event.hpp"
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

static bool handle_scroll(Event event, int& scroll_up) {
    if (event.is_mouse() && event.mouse().button == Mouse::WheelUp) {
        scroll_up += 3;
        return true;
    }
    if (event.is_mouse() && event.mouse().button == Mouse::WheelDown) {
        scroll_up = std::max(0, scroll_up - 3);
        return true;
    }
    if (event == Event::ArrowUp) {
        scroll_up += 1;
        return true;
    }
    if (event == Event::ArrowDown) {
        scroll_up = std::max(0, scroll_up - 1);
        return true;
    }
    if (event == Event::PageUp) {
        scroll_up += 5;
        return true;
    }
    if (event == Event::PageDown) {
        scroll_up = std::max(0, scroll_up - 5);
        return true;
    }
    return false;
}

void start() {
    auto session = chat_init();
    if (!session) {
        log(LogLevel::Error, "Chat session not initialized");
        return;
    } 

    auto screen = ScreenInteractive::Fullscreen();
    InputOption input_option;
    input_option.transform = [&](InputState state) {
        state.element |= color(session->theme.prompt_color);
        return state.element;
    };
    auto input_prompt = Input(&session->prompt.content, "Write something...", input_option);
    auto component = Container::Horizontal({
        input_prompt
    });

    auto final_component = component | CatchEvent([&](Event event) {
        if (event == Event::Escape) {
            if (session->busy) {
                session->cancelled = true;
                return true;
            }
            return false;
        }
        if (handle_scroll(event, session->scroll_up)) {
            screen.RequestAnimationFrame();
            return true;
        }

        if (!session->pending_command.empty()) {
            if (event == Event::Character("y")) {
                session->active_promise->set_value(true);
                session->pending_command.clear();
                screen.RequestAnimationFrame();
                return true;
            }
            if (event == Event::Character("n")) {
                session->active_promise->set_value(false);
                session->pending_command.clear();
                screen.RequestAnimationFrame();
                return true;
            }
            return true;
        }

        if (event == Event::Return) {
            if (session->busy) return true;
            if (session->worker.joinable()) session->worker.join();

            auto trimmed = session->prompt.content;
            trimmed.erase(trimmed.find_last_not_of(" \n\r\t") + 1);
            if (trimmed.empty()) {
                session->prompt.content.clear();
                return true;
            }
            session->prompt.content = trimmed;

            if (session->prompt.content.empty()) return false;
            if (session->prompt.content == "/exit") {
                screen.Exit();
                return true;
            }
            session->conversation.push_back({Role::User, session->prompt.content,
                "", {}});
            log(LogLevel::Info, "User prompted: " + session->prompt.content);
            session->prompt.content.clear();

            session->scroll_up = 0;
            session->cancelled = false;
            session->busy = true;
            session->worker = std::thread([&]{
                try {
                    auto local_conv = session->conversation;
                    auto output = session->account->send_request(local_conv,
                        [&](const std::string& delta) {
                        screen.Post([&, delta] {
                            session->streaming_buffer += delta;
                            screen.RequestAnimationFrame();
                        });
                    }, &session->cancelled);

                    while (!output.tool_calls.empty() && !session->cancelled) {
                        local_conv.push_back({Role::Assistant, "", "", output.tool_calls});
                        auto results = handle_tool_calls(output, [&](const std::string& cmd) {
                            std::promise<bool> promise;
                            auto future = promise.get_future();
                            screen.Post([&, cmd] {
                                session->pending_command = cmd;
                                session->active_promise = &promise;
                                screen.RequestAnimationFrame();
                            });
                            return future.get();
                        });
                        for (const auto& msg : results) {
                            local_conv.push_back(msg);
                        }
                        log(LogLevel::Debug, "Number of tool calls: "
                            + std::to_string(output.tool_calls.size()));
                        output.tool_calls.clear();
                        output = session->account->send_request(local_conv,
                            [&](const std::string& delta) {
                            screen.Post([&, delta] {
                                session->streaming_buffer += delta;
                                screen.RequestAnimationFrame();
                            });
                        }, &session->cancelled);
                        const std::string is_failed = output.is_failed ? "true" : "false";
                        log(LogLevel::Debug, "API returned output: " + output.content);
                        log(LogLevel::Debug, "Is API request failed: " + is_failed);
                        log(LogLevel::Debug, "Number of tool calls: "
                            + std::to_string(output.tool_calls.size()));
                    }

                    if (session->cancelled) {
                        log(LogLevel::Info, "Request cancelled by user");
                        auto partial = session->streaming_buffer;
                        screen.Post([&, local_conv, partial] {
                            session->conversation = local_conv;
                            if (!partial.empty()) {
                                session->conversation.push_back(
                                    {Role::Assistant, partial, "", {}});
                            }
                            session->streaming_buffer.clear();
                        });
                    } else if (output.is_failed) {
                        log(LogLevel::Error, "Request failed with error: " + output.content);
                        screen.Post([&] {
                            session->conversation.pop_back();
                            session->streaming_buffer.clear();
                        });
                    } else {
                        local_conv.push_back({Role::Assistant, output.content, "", {}});
                        if (session->limit > 0) {
                            while (local_conv.size() > session->limit) {
                                local_conv.erase(local_conv.begin() + 1);
                            }
                        }
                        screen.Post([&, local_conv] {
                            session->conversation = local_conv;
                            session->streaming_buffer.clear();
                        });
                    }
                    log(LogLevel::Debug, "Model's output (after tool call): " + output.content);
                    session->busy = false;
                } catch (const std::exception& e) {
                    log(LogLevel::Error, "Thread crashed: " + std::string(e.what()));
                    session->busy = false;
                }}
            );
            return true;
        }
        return false;
    });
    const auto& theme = session->theme;
    auto renderer = Renderer(final_component, [&] {
        Elements messages;
        for (const auto& msg : session->conversation) {
            if (msg.role == Role::User) {
                messages.push_back(vbox({
                    hbox(text("❯ ") | color(theme.prompt_color), paragraph(msg.content) | color(theme.user_color)),
                    text(""),
                }));
            } else if (msg.role == Role::Assistant) {
                messages.push_back(vbox({
                    paragraph(msg.content) | color(theme.assistant_color),
                    text(""),
                }));
            }
        }

        if (!session->streaming_buffer.empty()) {
            messages.push_back(vbox({
                paragraph(session->streaming_buffer) | color(theme.streaming_color),
                text(""),
            }));
        }

        if (!session->pending_command.empty()) {
            messages.push_back(
                text("Allow: " + session->pending_command + "? (y/n)") | color(theme.status_color));
        }

        if (!messages.empty()) {
            int target = static_cast<int>(messages.size()) - 1 - session->scroll_up;
            target = std::max(0, target);
            session->scroll_up = static_cast<int>(messages.size()) - 1 - target;
            messages[target] = messages[target] | focus;
        }

        auto input_line = hbox(text("❯ ") | color(theme.prompt_color), input_prompt->Render());
        if (session->busy) input_line = input_line | dim;

        return vbox({
            vbox(messages) | yframe | flex,
            separator() | color(theme.separator_color),
            input_line,
        });
    });

    screen.Loop(renderer);
    
    if (!session->pending_command.empty()) {
        session->active_promise->set_value(false);
    }
    if (session->worker.joinable()) session->worker.join();
    save_chat(session->chats_path, session->conversation);
}
