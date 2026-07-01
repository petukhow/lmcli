#include "chat_flow.h"
#include "json.hpp"
#include "constants.h"
#include "loaders/config.h"
#include "loaders/accounts.h"
#include "loaders/theme.h"
#include "providers/provider.h"
#include "logging/logger.h"
#include <memory>

using json = nlohmann::json;

std::unique_ptr<ChatSession> chat_init() {
    std::vector<Message> conversation;
    auto session = std::make_unique<ChatSession>();
    const auto config = load_config(CONFIG_FILE);
    const auto accounts = load_accounts(ACCOUNTS_FILE);

    if (config.is_null() || accounts.is_null()) {
        log(LogLevel::Error, "Failed to load config or accounts");
        session->startup_error = "Failed to load config or accounts.";
        return session;
    }

    const auto account_name = config["current_account"].get<std::string>();

    std::unique_ptr<Provider> account = nullptr;

    if (!account_name.empty()) {
        for (const auto& acc : accounts["accounts"]) {
            if (acc["name"] == account_name) {
                account = Provider::create(acc, config);
                break;
            }   
        }
    }

    const auto theme = load_theme(config["theme"].get<std::string>());

    session->account = std::move(account);
    session->chats_path = std::nullopt;
    session->conversation = std::move(conversation);
    session->limit = config["limit"];
    session->theme = std::move(theme);
    return session;
}
