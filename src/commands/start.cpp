#include "commands.h"
#include "json.hpp"
#include "select_account.h"
#include "constants.h"
#include "chats.h"
#include "config.h"
#include "providers.h"
#include "accounts.h"
#include "message.h"
#include <iostream>
#include "colors.h"

using json = nlohmann::json;

using namespace colors;

void start() {
    json config = load_config(CONFIG_FILE);
    json accounts = load_accounts(ACCOUNTS_FILE);
    json providers = load_providers();

    if (config.empty()) {
        std::cerr << "Config not found. Try 'lmcli init'.\n";
        return;
    }

    if (accounts.empty()) {
        std::cerr << "Accounts file not found. Try 'lmcli setup'.\n";
        return;
    }

    std::vector<Message> conversation;
    
    Message prompt;
    Message answer;

    auto account = select_account(accounts, config);
    if (!account) return;

    std::string chats_path = setup_chat();
    if (chats_path.empty()) {
        return;
    }

    json chats = load_chats(chats_path);

    if (chats.contains("conversation") && chats["conversation"].is_array()) {
        conversation = chats["conversation"].get<std::vector<Message>>();
    }

    if (!conversation.empty()) {
        if (conversation[0].content != config["system_prompt"]) {
            conversation[0].content = config["system_prompt"];
        }
    }

    size_t limit_messages = config["limit"].get<size_t>();
    if (conversation.empty()) {
        conversation.push_back({"system", config["system_prompt"].get<std::string>()});
    }

    std::cout << "\nPrompt (or '/exit' to end the conversation): \n";

    while (true) {
        std::cout << CYAN << "You: " << END;
        if (!std::getline(std::cin, prompt.content)) break; // user's prompt
        if (prompt.content == "/exit") break; 
        if (prompt.content == "") continue;
        conversation.push_back({"user", prompt.content});

        std::cout << YELLOW << "Model: " << END;
        answer = account->send_request(conversation);

        if (answer.is_failed) {
            std::cout << "Request failed: " << answer.content << "\n";
            conversation.pop_back();
            continue;
        }
        
        conversation.push_back({"assistant", answer.content});
        std::cout << "\n\n";

        if (limit_messages > 0) {
            while (conversation.size() > limit_messages) {
                conversation.erase(conversation.begin() + 1);
            }
        }
        save_chat(chats_path, conversation);
    }
}
