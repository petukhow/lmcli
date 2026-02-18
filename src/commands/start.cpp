#include "commands.h"
#include "json.hpp"
#include "selectAccount.h"
#include "constants.h"
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

    if (config.empty()) {
        std::cerr << "Config files not found. Run 'lmcli setup' to create.\n";
        return;
    }

    if (providers.empty()) {
        std::cerr << "Providers config doesn't exist. Your installation may be broken.\n";
        return;
    }

    auto provider = selectAccount(accounts, config);
    
    if (provider == nullptr) {
        std::cerr << "No provider selected.\n";
        return;
    }

    conversation.push_back({"system", config["system_prompt"].get<std::string>()});
    size_t limit_messages = config["limit"].get<size_t>();

    std::cout << "Prompt (or '/exit' to end the conversation): \n";

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, prompt.content);
        if (prompt.content == "/exit") break; 
        conversation.push_back({"user", prompt.content});

        answer = provider->sendRequest(conversation);

        if (answer.isFailed) {
            std::cout << "Request failed: " << answer.content << "\n";
            conversation.pop_back();
            continue;
        }
        conversation.push_back({"assistant", answer.content});

        std::cout << answer.content << "\n";

        if (limitExceeded(conversation, limit_messages)) {
            conversation.erase(conversation.begin() + 1); // erases user's message
            conversation.erase(conversation.begin() + 1); // erases model's answer
        }
    }
}
