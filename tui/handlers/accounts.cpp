#include "constants.h"
#include "chat_flow.h"
#include "logging/logger.h"
#include "loaders/config.h"
#include "loaders/accounts.h"
#include "json.hpp"

static std::unique_ptr<Provider> select_acc(const nlohmann::json& accounts_list, int cursor, nlohmann::json& config) {
    config["current_account"] = accounts_list[cursor]["name"].get<std::string>();
    save_config(config);
    return Provider::create(accounts_list[cursor], config);
}

void open_account_menu(ChatSession& session) {
    auto config = load_config(CONFIG_FILE);
    auto accounts = load_accounts(ACCOUNTS_FILE);

    if (!accounts.contains("accounts") || accounts["accounts"].empty()) {
        log(LogLevel::Error, "Broken or empty accounts.json config");
    }

    auto accounts_list = accounts["accounts"];

    if (accounts_list.size() == 1) {
        session.account = select_acc(accounts_list, 0, config);
        log(LogLevel::Info, "Account selected: " + accounts_list[0]["name"].get<std::string>());
    }

    session.menu_settings.menu_cursor = 0;
    session.menu_settings.menu_items.clear();
    session.menu_settings.title = "Select an account:";
    session.prompt.content.clear();
    for (const auto& account : accounts_list) {
        session.menu_settings.menu_items.push_back(account["name"].get<std::string>());
    }

    session.menu_settings.on_select = [&session, accounts_list, &config](int cursor) {
        session.account = select_acc(accounts_list, cursor, config);
        log(LogLevel::Info, "Account selected: " + accounts_list[cursor]["name"].get<std::string>());
    };

    session.mode = Mode::Menu;
}