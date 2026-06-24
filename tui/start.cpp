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
#include "loaders/config.h"
#include "loaders/json_io.h"
#include "providers/provider.h"
#include "providers/providers.h"
#include "types/accounts.h"
#include "render.h"
#include "tools/process_tool_calls.h"
#include "logging/logger.h"
#include "types/message.h"
#include "types/roles.h"
#include "accounts/build_account.h"
#include <filesystem>
#include <optional>
#include <vector>
#include <memory>
#include "utils/utils.h"
#include <iostream>

#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include <ftxui/component/screen_interactive.hpp>
#include "ftxui/component/event.hpp"
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>


using namespace ftxui;
using json = nlohmann::json;

static std::unique_ptr<Provider> select_acc(const json& accounts_list, int cursor, json& config) {
    config["current_account"] = accounts_list[cursor]["name"].get<std::string>();
    save_config(config);
    return Provider::create(accounts_list[cursor], config);
}

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

    auto form_name_input = Input(&session->account_draft.acc_name, "Account name");
    auto form_key_input = Input(&session->account_draft.api_key, "API key");
    auto form_model_input = Input(&session->account_draft.model_name, "Model");
    auto form_container = Container::Vertical({
        form_name_input, form_key_input, form_model_input
    });

    auto config_prompt_input = Input(&session->config_draft.system_prompt, "System prompt");
    auto config_limit_input = Input(&session->config_draft.limit, "Limit");
    auto config_tokens_input = Input(&session->config_draft.max_tokens, "Max tokens");
    auto config_logging_toggle = Checkbox("Logging", &session->config_draft.logging);
    auto config_confirm_input = Input(&session->config_draft.confirm_required_raw, "Commands which require confirmation (json array or 'all' value)");
    auto config_restricted_input = Input(&session->config_draft.restricted_raw, "Restricted commands (json array)");

    auto config_container = Container::Vertical({
        config_prompt_input,
        config_limit_input,
        config_tokens_input,
        config_logging_toggle,
        config_confirm_input,
        config_restricted_input,
    });

    auto chat_name_input = Input(&session->chat_name_input, "Chat name (empty for auto)");
    auto chat_name_container = Container::Vertical({chat_name_input});

    int active_tab = 0;
    auto component = Container::Tab({
        Container::Horizontal({input_prompt}),
        form_container,
        config_container,
        chat_name_container,
    }, &active_tab);


    auto open_setup = [&]() {
        const auto& providers = load_providers(PROVIDERS_FILE);
        if (providers.empty()) return;

        auto providers_list = providers["providers"];
        open_prov_menu(session, providers_list);

        session->menu_settings.on_select = [&active_tab, session = session.get(), providers_list](size_t cursor) {
            const auto& provider = providers_list[cursor];

            ProviderInfo provider_fields {
                provider["type"].get<std::string>(),
                provider["default_api_url"].get<std::string>(),
                provider["default_model"].get<std::string>(),
                provider["name"].get<std::string>(),
            };
            log(LogLevel::Info, "Provider for setup: " + provider_fields.default_name);

            session->form.on_submit = [session, provider_fields]() -> bool {
                auto& draft = session->account_draft;
                json accounts = load_accounts(ACCOUNTS_FILE);
                log(LogLevel::Info, "Accounts loaded");
                auto& accounts_list = accounts["accounts"];

                auto new_account = build_account(draft, provider_fields,
                    accounts_list);
                log(LogLevel::Debug, "New account builded");
                if (!new_account) return false;

                auto config = load_config(CONFIG_FILE);
                if (config["current_account"].get<std::string>().empty()) {
                    config["current_account"] = (*new_account)["name"].get<std::string>();
                    save_config(config);
                    session->account = Provider::create(*new_account, config);
                }

                accounts_list.push_back(*new_account);
                save_accounts(accounts);
                log(LogLevel::Info, "New account created: " + (*new_account)["name"].get<std::string>());
                
                return true;
            };
            session->mode = Mode::Form;
            active_tab = 1;
            return true;
        };

        session->mode = Mode::Menu;
    };

    if (!session->account) {
        open_setup();
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
                        auto config = load_config(CONFIG_FILE);

                        if (accounts_list.size() == 1) {
                            session->account = select_acc(accounts_list, 0, config);
                            log(LogLevel::Info, "Account selected: " + accounts_list[0]["name"].get<std::string>());
                            return true;
                        }

                        open_acc_menu(session, accounts_list);

                        session->menu_settings.on_select = [session = session.get(), accounts_list, &config](int cursor) {
                            session->account = select_acc(accounts_list, cursor, config);
                            log(LogLevel::Info, "Account selected: " + accounts_list[cursor]["name"].get<std::string>());
                            return true;
                        };

                        session->mode = Mode::Menu;
                        return true;
                    } 

                    if (session->active_form == "/setup") {
                        open_setup();
                        return true;
                    };

                    if (session->active_form == "/remove") {
                        session->menu_settings.menu_cursor = 0;
                        session->menu_settings.menu_items = {"Chat", "Chats", "Account"};
                        session->menu_settings.title = "What to remove?";

                        session->menu_settings.on_select = [session = session.get()](size_t cursor) {
                            if (cursor == 0) {
                                const std::string chats_dir = get_chats_dir();
                                auto chats = store_chats(chats_dir);
                                if (chats.empty()) {
                                    log(LogLevel::Info, "No chats to remove.");
                                    return;
                                }

                                session->menu_settings.menu_cursor = 0;
                                session->menu_settings.title = "Select a chat to remove:";
                                for (const auto& chat : chats) {
                                    session->menu_settings.menu_items.push_back(chat.path().stem().string());
                                }

                                session->menu_settings.on_select = [chats](size_t c) {
                                    std::filesystem::remove(chats[c].path());
                                    log(LogLevel::Info, "Chat removed: " + chats[c].path().stem().string());
                                };
                            } else if (cursor == 1) {
                                const std::string chats_dir = get_chats_dir();
                                auto chats = store_chats(chats_dir);
                                if (chats.empty()) {
                                    log(LogLevel::Info, "No chats to remove.");
                                    return;
                                }

                                session->menu_settings.menu_cursor = 0;
                                session->menu_settings.menu_items = {"Yes", "No"};
                                session->menu_settings.title = "WARNING: This will remove all chats.";

                                session->menu_settings.on_select = [chats_dir](size_t c) {
                                    if (c == 0) {
                                        for (const auto& chat : std::filesystem::directory_iterator(chats_dir)) {
                                            std::filesystem::remove(chat);
                                        }
                                        log(LogLevel::Info, "All chats removed successfully");
                                    }
                                };
                            } else if (cursor == 2) {
                                json accounts = load_accounts(ACCOUNTS_FILE);
                                if (!accounts.contains("accounts") || accounts["accounts"].empty()) {
                                    log(LogLevel::Error, "No accounts to remove.");
                                    return;
                                }

                                auto accounts_json = accounts["accounts"];
                                session->menu_settings.menu_cursor = 0;
                                session->menu_settings.title = "Select an account to remove:";
                                for (const auto& account : accounts_json) {
                                    session->menu_settings.menu_items.push_back(account["name"].get<std::string>());
                                }

                                session->menu_settings.on_select = [](size_t c) {
                                    json accounts = load_accounts(ACCOUNTS_FILE);
                                    auto& accounts_list = accounts["accounts"];
                                    if (c < accounts_list.size()) {
                                        std::string name = accounts_list[c]["name"].get<std::string>();
                                        accounts_list.erase(c);
                                        save_accounts(accounts);
                                        log(LogLevel::Info, "Account " + name + " removed.");
                                    }
                                };
                            }
                        };

                        session->mode = Mode::Menu;
                        return true;
                    }

                    if (session->active_form == "/chats") {
                        save_chat(session->chats_path, session->conversation);

                        const std::string chats_dir = get_chats_dir();
                        auto chats = store_chats(chats_dir);

                        session->menu_settings.menu_cursor = 0;
                        session->menu_settings.menu_items.clear();
                        session->menu_settings.title = "Select a chat:";
                        session->prompt.content.clear();

                        for (const auto& chat : chats) {
                            session->menu_settings.menu_items.push_back(chat.path().stem().string());
                        }
                        session->menu_settings.menu_items.push_back("New chat");

                        auto switch_chat = [session = session.get()](const std::string& path) {
                            auto chat_json = load_json(path);
                            std::vector<Message> conversation;
                            if (chat_json && chat_json->contains("conversation") && (*chat_json)["conversation"].is_array()) {
                                conversation = (*chat_json)["conversation"].get<std::vector<Message>>();
                            }

                            auto config = load_config(CONFIG_FILE);
                            if (!conversation.empty()) {
                                if (conversation[0].content != config["system_prompt"].get<std::string>()) {
                                    conversation[0].content = config["system_prompt"];
                                }
                            }
                            if (conversation.empty()) {
                                conversation.push_back({Role::System, config["system_prompt"].get<std::string>(), "", {}});
                            }

                            session->conversation = std::move(conversation);
                            session->chats_path = path;
                            session->scroll_pos = 1.0f;
                            session->error_message.clear();
                            session->streaming_buffer.clear();
                        };

                        session->menu_settings.on_select = [session = session.get(), chats, chats_dir, &active_tab, switch_chat](size_t cursor) {
                            if (cursor == chats.size()) {
                                session->chat_name_input.clear();
                                session->form.on_submit = [session, chats_dir, switch_chat]() -> bool {
                                    std::string name = session->chat_name_input;
                                    name.erase(0, name.find_first_not_of(" \t"));
                                    name.erase(name.find_last_not_of(" \t") + 1);

                                    if (name.empty()) {
                                        size_t files_amount = std::distance(
                                            std::filesystem::directory_iterator(chats_dir),
                                            std::filesystem::directory_iterator{});
                                        name = "chat #" + std::to_string(files_amount);
                                    }

                                    std::string new_path = chats_dir + name + ".json";
                                    create_file_if_not_exists(new_path, CHAT_DEFAULT);
                                    switch_chat(new_path);
                                    return true;
                                };
                                session->active_form = "/chats";
                                session->mode = Mode::Form;
                                active_tab = 3;
                            } else {
                                switch_chat(chats[cursor].path().string());
                            }
                        };

                        session->mode = Mode::Menu;
                        return true;
                    }

                    if (session->active_form == "/config") {
                        auto config = load_config(CONFIG_FILE);
                        session->config_draft.system_prompt = config["system_prompt"].get<std::string>();
                        session->config_draft.limit = std::to_string(config["limit"].get<size_t>());
                        session->config_draft.max_tokens = std::to_string(config["max_tokens"].get<size_t>());
                        session->config_draft.logging = config["logging"].get<bool>();
                        if (config["confirm_required"].is_string()) {
                            session->config_draft.confirm_required_raw = config["confirm_required"].get<std::string>();
                        } else {
                            session->config_draft.confirm_required_raw = config["confirm_required"].dump();
                        }
                        session->config_draft.restricted_raw = config["blacklist"].dump();

                        session->form.on_submit = [session = session.get()]() -> bool {
                            auto& draft = session->config_draft;
                            bool has_errors = false;
                            try {
                                int limit = std::stoi(draft.limit);
                                if (limit < 0) {
                                    draft.limit_error = "Limit must be a positive number";
                                    has_errors = true;
                                }
                                else draft.limit_error = std::nullopt;
                            } catch (const std::invalid_argument&) {
                                draft.limit_error = "Limit must be a number";
                                has_errors = true;
                            }

                            try {
                                int max_tokens = std::stoi(draft.max_tokens);
                                if (max_tokens < 0) {
                                    draft.max_tokens_error = "Max tokens must be a positive number";
                                    has_errors = true;
                                }
                                else draft.max_tokens_error = std::nullopt;
                            } catch (const std::invalid_argument&) {
                                draft.max_tokens_error = "Max tokens must be a number";
                                has_errors = true;
                            }

                            json confirm_json, blacklist_json;
                            if (draft.confirm_required_raw == "all") confirm_json = "all";
                            else {
                                try {
                                    confirm_json = json::parse(draft.confirm_required_raw);
                                    if (!confirm_json.is_array()) {
                                        draft.confirm_required_error = "Confirm required commands must be a JSON array";
                                        has_errors = true;
                                    }
                                } catch (const json::parse_error&) {
                                    draft.confirm_required_error = "Confirm required commands field has invalid JSON";
                                    has_errors = true;
                                }
                            }

                            try {
                                blacklist_json = json::parse(draft.restricted_raw);
                                if (!blacklist_json.is_array()) {
                                    draft.restricted_error = "Restricted commands must be a JSON array";
                                    has_errors = true;
                                }
                            } catch (const json::parse_error&) {
                                draft.restricted_error = "Restricted commands field has invalid JSON";
                                has_errors = true;
                            }

                            if (has_errors) return false;

                            json config = load_config(CONFIG_FILE);
                            config["system_prompt"] = draft.system_prompt;
                            config["limit"] = std::stoi(draft.limit);
                            config["max_tokens"] = std::stoi(draft.max_tokens);
                            config["logging"] = draft.logging;
                            config["confirm_required"] = confirm_json;
                            config["blacklist"] = blacklist_json;
                            save_config(config);

                            return true;
                        };

                        session->mode = Mode::Form;
                        active_tab = 2;
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
                    auto c = session->menu_settings.menu_cursor;
                    auto on_select = std::move(session->menu_settings.on_select);
                    session->menu_settings = {};
                    on_select(c);
                    if (session->menu_settings.menu_items.empty() && session->mode != Mode::Form) {
                        session->mode = Mode::Main;
                    }

                    screen.RequestAnimationFrame();
                }
                return true;
            case Mode::Form:
                if (event == Event::Escape) {
                    session->mode = Mode::Main;
                    active_tab = 0;
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
                        active_tab = 0;
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
                        active_tab = 0;
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
        
        if (session->mode == Mode::Form) {
            if (name_exists(accounts_list, session->account_draft.acc_name)) {
                session->account_draft.name_error = "Account with this name already exists";
            } else {
                session->account_draft.name_error = std::nullopt;
            }
        }

        Elements acc_errors;
        Elements conf_errors;
        if (session->active_form == "/setup") {
            if (session->account_draft.name_error.has_value())
                acc_errors.push_back(hbox({text(""), text(*session->account_draft.name_error) | color(Color::Yellow)}));
            if (session->account_draft.key_error.has_value())
                acc_errors.push_back(hbox({text(""), text(*session->account_draft.key_error) | color(Color::Red)}));
        }
        else if (session->active_form == "/config") {
            if (session->config_draft.limit_error.has_value())
                conf_errors.push_back(hbox({text(""), text(*session->config_draft.limit_error) | color(Color::Red)}));
            if (session->config_draft.max_tokens_error.has_value())
                conf_errors.push_back(hbox({text(""), text(*session->config_draft.max_tokens_error) | color(Color::Red)}));
            if (session->config_draft.confirm_required_error.has_value())
                conf_errors.push_back(hbox({text(""), text(*session->config_draft.confirm_required_error) | color(Color::Red)}));
            if (session->config_draft.restricted_error.has_value())
                conf_errors.push_back(hbox({text(""), text(*session->config_draft.restricted_error) | color(Color::Red)}));
        }

        auto acc_errors_block = vbox(std::move(acc_errors));
        auto conf_errors_block = vbox(std::move(conf_errors));

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
                if (session->active_form == "/setup") {
                    footer = vbox({
                        !session->account ? paragraph("You have no accounts. Create one to continue.") | color(Color::Yellow) : emptyElement(),
                        hbox({text("Account name") | size(WIDTH, EQUAL, 14) | color(session->theme.prompt_color), 
                                text("› ") | color(session->theme.prompt_color), form_name_input->Render()}),
                        hbox({text("API Key") | size(WIDTH, EQUAL, 14) | color(session->theme.prompt_color),
                                text("› ") | color(session->theme.prompt_color), form_key_input->Render()}),
                        hbox({text("Model") | size(WIDTH, EQUAL, 14) | color(session->theme.prompt_color),
                                text("› ") | color(session->theme.prompt_color), form_model_input->Render()}),
                        separator() | color(theme.separator_color),
                        acc_errors_block,
                        paragraph(" esc to exit") | color(Color::GrayDark),
                        text("")
                    });
                }
                else if (session->active_form == "/chats") {
                    footer = vbox({
                        hbox({text(" Chat name") | size(WIDTH, EQUAL, 11) | color(session->theme.prompt_color),
                            text("› ") | color(session->theme.prompt_color), chat_name_input->Render()}),
                        separator() | color(theme.separator_color),
                        paragraph(" enter to create | esc to cancel") | color(Color::GrayDark),
                        text("")
                    });
                }
                else if (session->active_form == "/config") {
                    footer = vbox({
                        hbox({text("System prompt") | size(WIDTH, EQUAL, 20) | color(session->theme.prompt_color),
                            text("› ") | color(session->theme.prompt_color), config_prompt_input->Render()}),
                        hbox({text("Limit") | size(WIDTH, EQUAL, 20) | color(session->theme.prompt_color),
                            text("› ") | color(session->theme.prompt_color), config_limit_input->Render()}),
                        hbox({text("Max tokens") | size(WIDTH, EQUAL, 20) | color(session->theme.prompt_color),
                            text("› ") | color(session->theme.prompt_color), config_tokens_input->Render()}),
                        hbox({text("Logging") | size(WIDTH, EQUAL, 20) | color(session->theme.prompt_color),
                            text("› ") | color(session->theme.prompt_color), config_logging_toggle->Render()}),
                        hbox({text("Confirm required") | size(WIDTH, EQUAL, 20) | color(session->theme.prompt_color),
                            text("› ") | color(session->theme.prompt_color), config_confirm_input->Render()}),
                        hbox({text("Restricted commands") | size(WIDTH, EQUAL, 20) | color(session->theme.prompt_color),
                            text("› ") | color(session->theme.prompt_color), config_restricted_input->Render()}),
                        separator() | color(theme.separator_color),
                        conf_errors_block,
                        text(""),
                        paragraph("ctrl + S to save | esc to exit") | color(Color::GrayDark),
                        text("")
                    });
                }
                break;
            }
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
