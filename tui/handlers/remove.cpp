#include "remove.h"
#include <string>
#include "chats.h"
#include "logging/logger.h"
#include "chat_flow.h"
#include "constants.h"
#include "loaders/accounts.h"
#include "utils/utils.h"
#include "json.hpp"

static void remove_chat(ChatSession& session) {
    const std::string chats_dir = get_chats_dir();
    auto chats = list_chats(chats_dir);
    if (chats.empty()) {
        log(LogLevel::Info, "No chats to remove.");
        return;
    }

    session.menu_settings.menu_cursor = 0;
    session.menu_settings.title = "Select a chat to remove:";
    for (const auto& chat : chats) {
        session.menu_settings.menu_items.push_back(chat.path().stem().string());
    }

    session.menu_settings.on_select = [chats](size_t c) {
        std::filesystem::remove(chats[c].path());
        log(LogLevel::Info, "Chat removed: " + chats[c].path().stem().string());
    };
}

static void remove_all_chats(ChatSession& session) {
    const std::string chats_dir = get_chats_dir();
    auto chats = list_chats(chats_dir);
    if (chats.empty()) {
        log(LogLevel::Info, "No chats to remove.");
        return;
    }

    session.menu_settings.menu_cursor = 0;
    session.menu_settings.menu_items = {"Yes", "No"};
    session.menu_settings.title = "WARNING: This will remove all chats.";

    session.menu_settings.on_select = [chats_dir](size_t c) {
        if (c == 0) {
            for (const auto& chat : std::filesystem::directory_iterator(chats_dir)) {
                std::filesystem::remove(chat);
            }
            log(LogLevel::Info, "All chats removed successfully");
        }
    };
}

static void remove_account(ChatSession& session) {
    nlohmann::json accounts = load_accounts(ACCOUNTS_FILE);
    if (!accounts.contains("accounts") || accounts["accounts"].empty()) {
        log(LogLevel::Error, "No accounts to remove.");
        return;
    }

    auto& accounts_json = accounts["accounts"];
    session.menu_settings.menu_cursor = 0;
    session.menu_settings.title = "Select an account to remove:";
    for (const auto& account : accounts_json) {
        session.menu_settings.menu_items.push_back(account["name"].get<std::string>());
    }

    session.menu_settings.on_select = [](size_t c) {
        nlohmann::json accounts = load_accounts(ACCOUNTS_FILE);
        auto& accounts_list = accounts["accounts"];
        if (c < accounts_list.size()) {
            std::string name = accounts_list[c]["name"].get<std::string>();
            accounts_list.erase(c);
            save_accounts(accounts);
            log(LogLevel::Info, "Account " + name + " removed.");
        }
    };
}

void open_remove_menu(ChatSession& session) {
    session.menu_settings.menu_cursor = 0;
    session.menu_settings.menu_items = {"Chat", "Chats", "Account"};
    session.menu_settings.title = "What to remove?";
    session.menu_settings.on_select = [&session](size_t cursor) {
        switch (cursor) {
            case 0: remove_chat(session); break;
            case 1: remove_all_chats(session); break;
            case 2: remove_account(session); break;
        }
    };
    session.mode = Mode::Menu;
}