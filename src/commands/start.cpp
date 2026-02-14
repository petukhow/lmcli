#include "commands.h"
#include "json.hpp"
#include "selectAccount.h"
#include "config.h"
#include "providers.h"
#include "accounts.h"
#include "message.h"
#include "utils.h"

using json = nlohmann::json;

void start() {
    json config = loadConfig("../config.json");
    json accounts = loadAccounts("../accounts.json");
    auto provider = selectAccount(accounts, config);
    std::vector<Message> conversation;
    Message prompt;
    Message answer;

    conversation.push_back({"system", config["system_prompt"].get<std::string>()});
    size_t limit_messages = config["limit"].get<size_t>();
    std::cout << "Prompt (or '/exit' to end the conversation): \n";
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, prompt.content);
        if (prompt.content == "/exit") break; 
        conversation.push_back({"user", prompt.content});
        
        try {
            answer = provider->sendRequest(conversation);
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }

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
}
