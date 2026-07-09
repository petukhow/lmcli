#include "constants.h"
#include "chat_flow.h"
#include "logging/logger.h"
#include "loaders/config.h"
#include "loaders/accounts.h"
#include "json.hpp"

static std::unique_ptr<Provider> select_acc(const nlohmann::json& accounts_list, int cursor, nlohmann::json config) {
    config["current_account"] = accounts_list[cursor]["name"].get<std::string>();
    save_config(config);
    return Provider::create(accounts_list[cursor], config);
}

void open_account_menu(ChatSession& session) {
    session.prompt.content.clear();
    auto accounts = load_accounts(ACCOUNTS_FILE);

    if (!accounts.contains("accounts") || accounts["accounts"].empty()) {
        log(LogLevel::Error, "Broken or empty accounts.json config");
    }

    auto& accounts_list = accounts["accounts"];

    if (accounts_list.size() == 1) {
        session.account = select_acc(accounts_list, 0, load_config(CONFIG_FILE));
        session.account_name = accounts_list[0]["name"].get<std::string>();
        log(LogLevel::Info, "Account selected: " + accounts_list[0]["name"].get<std::string>());
    }

    session.menu_settings.menu_cursor = 0;
    session.menu_settings.menu_items.clear();
    session.menu_settings.title = "Select an account:";
    session.prompt.content.clear();
    for (const auto& account : accounts_list) {
        session.menu_settings.menu_items.push_back(account["name"].get<std::string>());
    }

    session.menu_settings.on_select = [&session, accounts_list](int cursor) {
        session.account = select_acc(accounts_list, cursor, load_config(CONFIG_FILE));
        session.account_name = accounts_list[cursor]["name"].get<std::string>();
        log(LogLevel::Info, "Account selected: " + accounts_list[cursor]["name"].get<std::string>());
    };

    session.menu_settings.on_delete = [&session](size_t cursor) {
        auto accounts = load_accounts(ACCOUNTS_FILE);
        auto& accounts_list = accounts["accounts"];
        if (cursor >= accounts_list.size()) return;

        const std::string deleted_name = accounts_list[cursor]["name"].get<std::string>();
        accounts_list.erase(accounts_list.begin() + static_cast<long>(cursor));
        save_accounts(accounts);

        if (session.account_name == deleted_name) {
            session.account.reset();
            session.account_name.clear();
            auto config = load_config(CONFIG_FILE);
            config["current_account"] = "";
            save_config(config);
        }

        session.menu_settings.menu_items.erase(session.menu_settings.menu_items.begin() + static_cast<long>(cursor));
        auto& c = session.menu_settings.menu_cursor;
        if (c >= session.menu_settings.menu_items.size() && c > 0) c--;

        log(LogLevel::Info, "Account removed: " + deleted_name);
    };

    session.mode = Mode::Menu;
}