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

#include "json.hpp"
#include "loaders/accounts.h"
#include "providers/provider.h"
#include "types/accounts.h"
#include "render.h"
#include "tools/process_tool_calls.h"
#include "logging/logger.h"
#include "types/message.h"
#include "types/roles.h"
#include <optional>
#include <vector>
#include <memory>
#include "utils/utils.h"
#include <iostream>

#include "handlers/remove.h"
#include "handlers/accounts.h"
#include "handlers/config.h"
#include "handlers/chats.h"
#include "handlers/setup.h"

#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include <ftxui/component/screen_interactive.hpp>
#include "ftxui/component/event.hpp"
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>


using namespace ftxui;
using json = nlohmann::json;

static std::vector<Message> confirm_tool_calls(const Message& output,
    ftxui::ScreenInteractive& screen, const std::unique_ptr<ChatSession>& session) {
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
    if (!session->startup_error.empty()) {
        std::cerr << "Error: " << session->startup_error << "\n";
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

    SetupFields setup_fields {
        Input(&session->account_draft.acc_name, "Account name"),
        Input(&session->account_draft.api_key, "API key"),
        Input(&session->account_draft.model_name, "Model"),
        Container::Vertical({
            setup_fields.form_name_input, setup_fields.form_key_input, setup_fields.form_model_input
        }),
    };

    ConfigFields config_fields {
        Input(&session->config_draft.system_prompt, "System prompt"),
        Input(&session->config_draft.limit, "Limit"),
        Input(&session->config_draft.max_tokens, "Max tokens"),
        Checkbox("Logging", &session->config_draft.logging),
        Input(&session->config_draft.confirm_required_raw, "Commands which require confirmation (json array or 'all' value)"),
        Input(&session->config_draft.restricted_raw, "Restricted commands (json array)"),
        nullptr,
    };
    config_fields.config_container = Container::Vertical({
        config_fields.config_prompt_input,
        config_fields.config_limit_input,
        config_fields.config_tokens_input,
        config_fields.config_logging_toggle,
        config_fields.config_confirm_input,
        config_fields.config_restricted_input,
    });

    auto chat_name_input = Input(&session->chat_name_input, "Chat name (empty for auto)");
    auto chat_name_container = Container::Vertical({chat_name_input});

    session->active_tab = 0;
    auto component = Container::Tab({
        Container::Horizontal({input_prompt}),
        setup_fields.form_container,
        config_fields.config_container,
        chat_name_container,
    }, &session->active_tab);

    if (!session->account) {
        open_setup_menu(*session);
    }

    auto accounts = load_accounts(ACCOUNTS_FILE);

    if (!accounts.contains("accounts") || accounts["accounts"].empty()) {
        log(LogLevel::Error, "Broken or empty accounts.json config");
    }

    auto accounts_list = accounts["accounts"];

    if (!session) {
        log(LogLevel::Error, "Chat session not initialized");
        return;
    } 

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
                    session->active_form = session->prompt.content;
                    if (session->busy) return true;
                    if (session->worker.joinable()) session->worker.join();

                    if (session->active_form == "/account") {
                        open_account_menu(*session);
                        return true;
                    } 

                    if (session->active_form == "/setup") {
                        open_setup_menu(*session);
                        return true;
                    };

                    if (session->active_form == "/remove") {
                        open_remove_menu(*session);
                        return true;
                    }

                    if (session->active_form == "/chats") {
                        open_chats_menu(*session);
                        return true;
                    }

                    if (session->active_form == "/config") {
                        open_config_menu(*session);
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
                        if (session->chats_path) {
                            close_chat(*session);
                        } else {
                            screen.Exit();
                        }
                        session->prompt.content.clear();
                        screen.RequestAnimationFrame();
                        return true;
                    }

                    if (!session->chats_path) {
                        auto created = create_chat(get_chats_dir());
                        open_chat(*session, created);
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
                    auto c = session->menu_settings.menu_cursor;
                    auto on_select = std::move(session->menu_settings.on_select);
                    session->menu_settings = {};
                    on_select(c);
                    if (session->menu_settings.menu_items.empty() && session->mode != Mode::Form) {
                        session->mode = Mode::Main;
                    }
                    screen.RequestAnimationFrame();
                    if (session->menu_settings.menu_items.empty() && session->mode != Mode::Form) {
                        session->mode = Mode::Main;
                    }

                    screen.RequestAnimationFrame();
                }
                return true;
            case Mode::Form:
                if (event == Event::Escape) {
                    session->mode = Mode::Main;
                    session->active_tab = 0;
                    screen.RequestAnimationFrame();
                    return true;
                }
                if (event == Event::Return && session->active_form == "/chats") {
                    auto ok = session->form.on_submit();
                    if (ok) {
                        session->chat_name_input.clear();
                        session->form = {};
                        session->prompt.content.clear();
                        session->mode = Mode::Main;
                        session->active_tab = 0;
                        screen.RequestAnimationFrame();
                        return true;
                    }
                }
                if (event == Event::CtrlS) {
                    if (session->active_form == "/setup") {
                        if (session->account_draft.api_key.empty()) {
                            session->account_draft.key_error = "API key cannot be empty";
                            return true;
                        }
                    }

                    auto ok = session->form.on_submit();
                    if (ok) {
                        session->config_draft = {};
                        session->account_draft = {};
                        session->form = {};
                        session->prompt.content.clear();
                        session->mode = Mode::Main;
                        session->active_tab = 0;
                        screen.RequestAnimationFrame();
                        return true;
                    }                    
                }
                return false;
        }
        return false;
    });
    auto renderer = Renderer(final_component, [&] {
        InputOption config_inputs;
        config_inputs.transform = [&](InputState state) {
            if (state.focused) {
                state.element |= color(session->theme.prompt_color);
            }
            return state.element;
        };
        
        auto acc_errors_block = build_acc_errors_block(*session, accounts_list);
        auto conf_errors_block = build_conf_errors_block(*session);

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
                    session->busy 
                        ? input_line | dim
                        : input_line,
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

            case Mode::Form: {
                if (session->active_form == "/setup" || !session->account) {
                    footer = render_setup_form(*session, setup_fields, acc_errors_block);
                }
                else if (session->active_form == "/chats") {
                    footer = render_chats_form(*session, chat_name_input);
                }
                else if (session->active_form == "/config") {
                    footer = render_config_form(*session, config_fields, conf_errors_block);
                }
                break;
            }
        }

        return vbox({
            session->chats_path ? vbox(messages) | focusPositionRelative(0, session->scroll_pos) | yframe | flex
                : paragraph("Hello, world!") | color(session->theme.prompt_color),
            separator() | color(theme.separator_color),
            footer
        });
    });

    screen.Loop(renderer);
    
    if (!session->pending_command.empty()) {
        session->active_promise->set_value(false);
    }
    if (session->worker.joinable()) session->worker.join();
    
    if (session->chats_path.has_value())
        close_chat(*session);
}
