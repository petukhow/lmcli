#include "../include/config.h"
#include "../include/providers.h"
#include "../include/accounts.h"
#include "../include/message.h"
#include "../include/utils.h"
#include "../include/json.hpp"
#include "../include/selectaccount.h"
#include <curl/curl.h>
#include <iostream>
#include <cstddef>
#include <string>
#include <vector>

using json = nlohmann::json;

int main() {
    json config = loadConfig("../config.json");
    json providers = loadProviders("../providers.json");
    json accounts = loadAccounts("../accounts.json");
    auto provider = selectAccount(accounts, config);
    std::vector<Message> conversation;
    Message prompt;
    Message answer;

    conversation.push_back({"system", config["system_prompt"].get<std::string>()});
    size_t limit_messages = config["limit"].get<size_t>();

    while (true) {
        std::cout << "Prompt (or '/exit' to end the conversation): ";
        std::getline(std::cin, prompt.content);
        if (prompt.content == "/exit") break; 
        conversation.push_back({"user", prompt.content});

        answer = provider->sendRequest(conversation);
        if (answer.content == "") {
            std::cout << "Something went wrong. Try again." << "\n";
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

    return 0;
}