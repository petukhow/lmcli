#include "chat_flow.h"
#include "accounts.h"
#include "chats.h"
#include "setup.h"
#include "config.h"
#include "theme.h"
#include <unordered_map>

static std::unordered_map<std::string, void(*)(ChatSession&)> commands =
    {
        {"/account", open_account_menu},
        {"/setup", open_setup_menu},
        {"/chats", open_chats_menu},
        {"/config", open_config_menu},
        {"/theme", open_theme_menu},
    };

bool dispatch_commands(ChatSession& session) {
    if (auto it = commands.find(session.active_form); it != commands.end()) {
        it->second(session);
        return true;
    }
    return false;
}