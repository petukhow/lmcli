/*
thread invariants:
    1. every single editing of shared data must be handled using screen.Post() (UI thread only)
    2. every waitings (future.get()) are only allowed in working thread (if UI thread sleeps = bad UX)
    3. only one working thread. busy flag must be initialized before thread
    4. active_promise is only valid when pending_command not empty (set_value only with this check) 
    5. a data of the worker thread must be handled by copy in screen.Post lambda; session - by reference
*/

#include "chat_flow.h"
#include "chats.h"
#include "commands.h"
#include "constants.h"
#include "ftxui/dom/elements.hpp"
#include "json.hpp"
#include "loaders/accounts.h"
#include "loaders/config.h"
#include "providers/provider.h"
#include "render.h"
#include "tools/handle_tool_calls.h"
#include "logging/logger.h"
#include "types/message.h"
#include "types/roles.h"

#include "ftxui/component/component.hpp"
#include <algorithm>
#include <ftxui/component/screen_interactive.hpp>
#include "ftxui/component/event.hpp"
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <memory>
#include <vector>

using namespace ftxui;

static std::unique_ptr<Provider> select_acc(const nlohmann::json& accounts_list, int cursor, const nlohmann::json& config) {
    return Provider::create(accounts_list[cursor], config);
}

static Element render_menu(const std::unique_ptr<ChatSession>& cs) {
    Elements lines;
    for (size_t i = 0; i < cs->menu_settings.menu_items.size(); ++i) {
        auto line = text(cs->menu_settings.menu_items[i]);
        if (cs->menu_settings.menu_cursor == i) {
            line = hbox(text("› "), line) | color(cs->theme.prompt_color);
        }
        else {
            line = hbox(text("› "), line) | dim;
        }
        lines.push_back(line);
    }
    return vbox({
        text("Select an account:"),
        vbox(lines),
        text("")
    });
}

static void open_menu(const std::unique_ptr<ChatSession>& cs, const nlohmann::json& accounts_list) {
    cs->menu_settings.menu_cursor = 0;
    cs->menu_settings.menu_items.clear();
    cs->prompt.content.clear();
    for (const auto& account : accounts_list) {
        cs->menu_settings.menu_items.push_back(account["name"].get<std::string>());
    }
}

static bool handle_scroll(Event event, float& scroll_pos) {
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

static std::vector<Message> confirm_tool_calls(const Message& output,
    ScreenInteractive& screen, const std::unique_ptr<ChatSession>& session) {
    return handle_tool_calls(output, [&](const std::string& cmd) {
            std::promise<bool> promise;
            auto future = promise.get_future();
            screen.Post([&, cmd] {
                session->pending_command = cmd;
                session->active_promise = &promise;
                screen.RequestAnimationFrame();
            });
            return future.get();
        });
}

// A second thread for core logic in UI function (API requests, tool calls handling, context management)
static void worker(const std::unique_ptr<ChatSession>& session, ScreenInteractive& screen) {
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
        auto results = confirm_tool_calls(output, screen, session);

        for (const auto& msg : results) local_conv.push_back(msg);

        log(LogLevel::Debug, "Number of tool calls: " + std::to_string(output.tool_calls.size()));

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
        log(LogLevel::Debug, "Number of tool calls: " + std::to_string(output.tool_calls.size()));
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
            screen.RequestAnimationFrame();
        });
    } else if (output.is_failed) {
        log(LogLevel::Error, "Request failed with error: " + output.content);
        auto err = output.content;
        screen.Post([&, err] {
            std::string failed_prompt = session->conversation.back().content;
            session->conversation.pop_back();
            session->prompt.content = failed_prompt;
            session->error_message = err;
            session->streaming_buffer.clear();
            screen.RequestAnimationFrame();
        });
    } else {
        local_conv.push_back({Role::Assistant, output.content, "", {}});
        trim_history(local_conv, session->limit);
        screen.Post([&, local_conv] {
            session->conversation = local_conv;
            session->streaming_buffer.clear();
            screen.RequestAnimationFrame();
        });
    }
    log(LogLevel::Debug, "Model's output (after tool call): " + output.content);
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
    auto input_prompt = Input(&session->prompt.content,
        "Write something...", input_option);
    auto component = Container::Horizontal({
        input_prompt
    });

    Element footer;
    const auto& theme = session->theme;
    auto final_component = component | CatchEvent([&](Event event) {
        switch (session->mode) {
            case Mode::Main:
                screen.RequestAnimationFrame();
                if (event == Event::Escape) {
                    if (session->busy) {
                        session->cancelled = true;
                        screen.RequestAnimationFrame();
                        return true;
                    }
                    return false;
                }

                if (handle_scroll(event, session->scroll_pos)) {
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
                    if (session->prompt.content == "/account") {
                        auto accounts = load_accounts(ACCOUNTS_FILE);
                        auto config = load_config(CONFIG_FILE);

                        if (!accounts.contains("accounts") || accounts["accounts"].empty()) {
                            log(LogLevel::Error, "Broken accounts.json config");
                            return true;
                        }
                        
                        auto accounts_list = accounts["accounts"];

                        if (accounts_list.size() == 1) {
                            session->account = select_acc(accounts_list, 0, config);
                            log(LogLevel::Info, "Account selected: " + accounts_list[0]["name"].get<std::string>());
                            return true;
                        }

                        open_menu(session, accounts_list);

                        session->menu_settings.on_select = [session = session.get(), accounts_list, config](size_t cursor) {
                            session->account = select_acc(accounts_list, cursor, config);
                            log(LogLevel::Info, "Account selected: " + accounts_list[cursor]["name"].get<std::string>());
                            return true;
                        };

                        session->mode = Mode::Menu;
                        return true;
                    } 

                    auto trimmed = session->prompt.content;
                    trimmed.erase(trimmed.find_last_not_of(" \n\r\t") + 1);
                    if (trimmed.empty()) {
                        session->prompt.content.clear();
                        screen.RequestAnimationFrame();
                        return true;
                    }
                    session->prompt.content = trimmed;
                    session->error_message.clear();

                    if (session->prompt.content.empty()) return false;
                    if (session->prompt.content == "/exit") {
                        screen.Exit();
                        return true;
                    }
                    session->conversation.push_back({Role::User, session->prompt.content,
                        "", {}});
                    log(LogLevel::Info, "User prompted: " + session->prompt.content);
                    session->prompt.content.clear();

                    session->scroll_pos = 1.0f;
                    session->cancelled = false;
                    session->busy = true;
                    session->worker = std::thread([&]{
                        try {
                            worker(session, screen);
                            session->busy = false;
                        } catch (const std::exception& e) {
                            log(LogLevel::Error, "Thread crashed: " + std::string(e.what()));
                            session->busy = false;
                        }}
                    );
                    return true;
                }
                return false;
            case Mode::Menu:
                if (event == Event::Escape) {
                    session->menu_settings = {};
                    session->mode = Mode::Main;
                }
                if (event == Event::ArrowDown) {
                    auto& c = session->menu_settings.menu_cursor;
                    if (c + 1 < session->menu_settings.menu_items.size()) ++c;
                    screen.RequestAnimationFrame();
                }
                if (event == Event::ArrowUp) {
                    auto& c = session->menu_settings.menu_cursor;
                    if (c > 0) --c;
                    screen.RequestAnimationFrame();
                }
                if (event == Event::Return) {
                    auto& c = session->menu_settings.menu_cursor;
                    session->menu_settings.on_select(c);
                    session->menu_settings = {};
                    screen.RequestAnimationFrame();
                    session->mode = Mode::Main;
                }
                return true;
            case Mode::Form:

                return true;
        }
        return false;
    });
    auto renderer = Renderer(final_component, [&] {
        Elements messages;
        for (const auto& msg : session->conversation) {
            if (msg.role == Role::User) {
                messages.push_back(vbox({
                    hbox({
                        text("› ") | color(theme.prompt_color),
                        paragraph(msg.content) | color(theme.user_color) | xflex,
                    }),
                    text(""),
                }));
            } else if (msg.role == Role::Assistant) {
                messages.push_back(vbox({
                    paragraph(msg.content) | color(theme.assistant_color) | xflex,
                    text(""),
                }));
            }
        }

        if (!session->streaming_buffer.empty()) {
            messages.push_back(vbox({
                paragraph(session->streaming_buffer) | color(theme.streaming_color) | xflex,
                text(""),
            }));
        }

        if (!session->pending_command.empty()) {
            messages.push_back(
                paragraph("Allow: " + session->pending_command + "? (y/n)")
                    | color(theme.status_color) | xflex);
        }

        switch (session->mode) {
            case Mode::Main: {                  
                auto input_line = hbox({
                    text("› ") | color(Color::GreenLight),
                    input_prompt->Render() | xflex,
                });
                footer = vbox({
                    !session->error_message.empty()
                        ? paragraph("✗ " + session->error_message) | color(Color::Red)
                        : emptyElement(),
                    input_line,
                    separator() | color(theme.separator_color),
                    session->busy
                        ? paragraph(" esc to interrupt") | color(Color::GrayDark)
                        : emptyElement(),
                });
                break;
            }   

            case Mode::Menu: 
                footer = render_menu(session);
                break;

            case Mode::Form:
                footer = emptyElement();
                break;
        }

        return vbox({
            vbox(messages) | focusPositionRelative(0, session->scroll_pos) | yframe | flex,
            separator() | color(theme.separator_color),
            footer
        });
    });

    screen.Loop(renderer);
    
    if (!session->pending_command.empty()) {
        session->active_promise->set_value(false);
    }
    if (session->worker.joinable()) session->worker.join();
    save_chat(session->chats_path, session->conversation);
}
