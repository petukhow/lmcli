#include "commands.h"
#include "json.hpp"
#include "selectAccount.h"
#include "constants.h"
#include "chats.h"
#include "config.h"
#include "providers.h"
#include "accounts.h"
#include "message.h"
#include "utils.h"
#include <iostream>

using json = nlohmann::json;

void start() {
    json config = loadConfig(CONFIG_FILE);
    json accounts = loadAccounts(ACCOUNTS_FILE);
    json providers = loadProviders();

    std::vector<Message> conversation;
    
    Message prompt;
    Message answer;

    auto account = selectAccount(accounts, config);
    if (!account) return;
        
    std::string chatsPath = setupChat();
    if (chatsPath.empty()) {
        return;
    }

    json chats = loadChats(chatsPath);
    if (chats.contains("conversation") && chats["conversation"].is_array()) {
        conversation = chats["conversation"].get<std::vector<Message>>();
    }

    if (conversation.empty()) {
        conversation.push_back({"system", config["system_prompt"].get<std::string>()});
    }
    size_t limit_messages = config["limit"].get<size_t>();

    std::cout << "Prompt (or '/exit' to end the conversation): \n";

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, prompt.content)) break; // user's prompt
        if (prompt.content == "/exit") break; 
        conversation.push_back({"user", prompt.content});

        answer = account->sendRequest(conversation);

        if (answer.isFailed) {
            std::cout << "Request failed: " << answer.content << "\n";
            conversation.pop_back();
            continue;
        }
        
        conversation.push_back({"assistant", answer.content});

        std::cout << "\n";
        std::cout << answer.content << "\n\n";

        if (limitExceeded(conversation, limit_messages)) {
            conversation.erase(conversation.begin() + 1); // erases user's first conversation message
            conversation.erase(conversation.begin() + 1); // erases model's first conversation answer 
        }
        saveChat(chatsPath, conversation);
    }
}
