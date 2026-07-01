#include "constants.h"
#include "chat_flow.h"
#include "providers/providers.h"
#include "render.h"
#include "loaders/config.h"
#include "loaders/accounts.h"
#include "logging/logger.h"
#include "accounts/build_account.h"
#include <ftxui/dom/elements.hpp>

void open_setup_menu(ChatSession& session) {
    const auto& providers = load_providers(PROVIDERS_FILE);
    session.mode = Mode::Main;
    if (providers.empty()) return;

    auto providers_list = providers["providers"];
    session.menu_settings.menu_cursor = 0;
    session.menu_settings.menu_items.clear();
    session.menu_settings.title = session.startup_error + "\nSelect a provider:";
    session.prompt.content.clear();
    for (const auto& provider : providers_list) {
        session.menu_settings.menu_items.push_back(provider["name"].get<std::string>());
    }

    session.menu_settings.on_select = [&session, providers_list](size_t cursor) {
        const auto& provider = providers_list[cursor];

        ProviderInfo provider_fields {
            provider["type"].get<std::string>(),
            provider["default_api_url"].get<std::string>(),
            provider["default_model"].get<std::string>(),
            provider["name"].get<std::string>(),
        };
        log(LogLevel::Info, "Provider for setup: " + provider_fields.default_name);

        session.form.on_submit = [&session, provider_fields]() -> bool {
            auto& draft = session.account_draft;
            auto accounts = load_accounts(ACCOUNTS_FILE);
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
                session.account = Provider::create(*new_account, config);
            }

            accounts_list.push_back(*new_account);
            save_accounts(accounts);
            log(LogLevel::Info, "New account created: " + (*new_account)["name"].get<std::string>());
            
            return true;
        };
        session.mode = Mode::Form;
        session.active_tab = 1;
    };
    session.mode = Mode::Menu;
}