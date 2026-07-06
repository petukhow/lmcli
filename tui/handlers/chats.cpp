#include "handlers/chats.h"
#include "../chats.h"
#include "chat_flow.h"
#include "utils/utils.h"
#include "loaders/json_io.h"
#include "loaders/config.h"
#include "constants.h"
#include "logging/logger.h"
#include <filesystem>

#include "ftxui/dom/elements.hpp"
#include <ftxui/screen/color.hpp>

void open_chat(ChatSession& cs, const std::string& path) {
    auto chat_json = load_json(path);
    std::vector<Message> conversation;
    if (chat_json && chat_json->contains("conversation") && (*chat_json)["conversation"].is_array()) {
        conversation = (*chat_json)["conversation"].get<std::vector<Message>>();
    }

    auto config = load_config(CONFIG_FILE);
    if (!conversation.empty()) {
        if (conversation[0].role == Role::System && conversation[0].content != config["system_prompt"].get<std::string>()) {
            conversation[0].content = config["system_prompt"];
        }
    }
    if (conversation.empty()) {
        conversation.push_back({Role::System, config["system_prompt"].get<std::string>(), "", {}});
    }

    cs.conversation = std::move(conversation);
    cs.chats_path = path;
    cs.scroll_pos = 1.0f;
    cs.error_message.clear();
    cs.streaming_buffer.clear();
}

void close_chat(ChatSession& cs) {
    if (cs.chats_path.has_value()) {
        save_chat(*cs.chats_path, cs.conversation);
    }
    cs.chats_path.reset();
}

void open_chats_menu(ChatSession& session) {
    if (session.chats_path.has_value())
        save_chat(*session.chats_path, session.conversation);

    const std::string chats_dir = get_chats_dir();
    auto chats = list_chats(chats_dir);

    session.menu_settings.menu_cursor = 0;
    session.menu_settings.menu_items.clear();
    session.menu_settings.title = "Select a chat:";
    session.prompt.content.clear();

    for (const auto& chat : chats) {
        session.menu_settings.menu_items.push_back(chat.path().stem().string());
    }

    session.menu_settings.on_select = [&session, chats, chats_dir](size_t cursor) {
        close_chat(session);
        open_chat(session, chats[cursor].path().string());
    };

    session.menu_settings.on_delete = [&session, chats_dir](size_t cursor) {
        auto chats = list_chats(chats_dir);
        if (cursor >= chats.size()) return;

        const std::string deleted_path = chats[cursor].path().string();
        const std::string deleted_stem = chats[cursor].path().stem().string();

        if (session.chats_path.has_value() && *session.chats_path == deleted_path) {
            session.chats_path.reset();
            session.conversation.clear();
        }

        std::filesystem::remove(deleted_path);

        session.menu_settings.menu_items.erase(session.menu_settings.menu_items.begin() + static_cast<long>(cursor));
        auto& c = session.menu_settings.menu_cursor;
        if (c >= session.menu_settings.menu_items.size() && c > 0) c--;

        log(LogLevel::Info, "Chat removed: " + deleted_stem);
    };

    session.mode = Mode::Menu;
}

using namespace ftxui;
Element render_chats_form(const ChatSession& session, const Component& chat_name_input) {
    return vbox({
        hbox({text(" Chat name") | size(WIDTH, EQUAL, 11) | color(session.theme.prompt_color),
            text("› ") | color(session.theme.prompt_color), chat_name_input->Render()}),
        separator() | color(session.theme.separator_color),
        paragraph("enter to create | esc to cancel") | color(Color::GrayDark),
        text("")
    });
}