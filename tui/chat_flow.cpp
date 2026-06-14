#include "chat_flow.h"
#include "json.hpp"
#include "select_account.h"
#include "constants.h"
#include "chats.h"
#include "loaders/config.h"
#include "loaders/accounts.h"
#include "loaders/json_io.h"
#include "loaders/theme.h"
#include "types/roles.h"
#include "logging/logger.h"

using json = nlohmann::json;

std::unique_ptr<ChatSession> chat_init() {
    std::vector<Message> conversation;
    const auto config = load_config(CONFIG_FILE);
    const auto accounts = load_accounts(ACCOUNTS_FILE);

    if (config.is_null() || accounts.is_null()) {
        log(LogLevel::Error, "Failed to load config or accounts");
        return nullptr;
    }

    auto account = select_account(accounts, config);
    if (!account) {
        return nullptr;
    }

    const std::string chats_path = setup_chat();
    if (chats_path.empty()) return nullptr;

    const auto chats = load_json(chats_path);

    if (!chats.has_value()) {
        return nullptr;
    }

    if (chats->contains("conversation") && (*chats)["conversation"].is_array()) {
        conversation = (*chats)["conversation"].get<std::vector<Message>>();
    }

    if (!conversation.empty()) {
        if (conversation[0].content != config["system_prompt"].get<std::string>()) {
            conversation[0].content = config["system_prompt"];
        }
    }

    if (conversation.empty()) {
        conversation.push_back({Role::System, config["system_prompt"].get<std::string>(), "", {}});
    }

    const auto theme = load_theme(config["theme"].get<std::string>());

    auto session = std::make_unique<ChatSession>();
    session->conversation = std::move(conversation);
    session->account = std::move(account);
    session->chats_path = std::move(chats_path);
    session->limit = config["limit"];
    session->theme = theme;
    return session;
}
