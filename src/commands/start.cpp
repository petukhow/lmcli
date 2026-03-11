#include "commands.h"
#include "json.hpp"
#include "provider.h"
#include "select_account.h"
#include "constants.h"
#include "chats.h"
#include "config.h"
#include "accounts.h"
#include "message.h"
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "colors.h"
#include "tools.h"

using json = nlohmann::json;

using namespace colors;

struct ChatValues {
    std::vector<Message> conversation;
    std::unique_ptr<Provider> account;
    std::string chats_path;
    size_t limit;
};

static void handle_tool_calls(const Message& output, std::vector<Message>& conversation) {
    for (const auto& tool : output.tool_calls) {
        if (tool.name == "read_file") {
            auto args = nlohmann::json::parse(tool.arguments);
            std::string path = args["file"];

            std::string result = read_file(path);

            conversation.push_back({"tool", result, tool.id, {}});
        }
    }
}

static std::optional<ChatValues> chat_init() {
    std::vector<Message> conversation;
    json config = load_config(CONFIG_FILE);
    json accounts = load_accounts(ACCOUNTS_FILE);

    std::string chats_path = setup_chat();
    if (chats_path.empty()) return std::nullopt;

    json chats = load_chats(chats_path);

    auto account = select_account(accounts, config);
    if (!account) return std::nullopt;

    if (chats.contains("conversation") && chats["conversation"].is_array()) {
        conversation = chats["conversation"].get<std::vector<Message>>();
    }

    if (!conversation.empty()) {
        if (conversation[0].content != config["system_prompt"]) {
            conversation[0].content = config["system_prompt"];
        }
    }

    if (conversation.empty()) {
        conversation.push_back({"system", config["system_prompt"].get<std::string>(), "", {}});
    }

    return ChatValues{std::move(conversation), std::move(account), chats_path, config["limit"]};
}

void start() {
    Message prompt;
    Message output; 

    auto values = chat_init(); // initializes the conversation vector
    if (!values) return;

    std::cout << "\nPrompt (or '/exit' to end the conversation): \n";
    while (true) {
        std::cout << CYAN << "You: " << END;
        if (!std::getline(std::cin, prompt.content)) break; // user's prompt
        if (prompt.content == "") continue;

        if (prompt.content[0] == '/') {
            if (prompt.content == "/exit") break; 
            
            size_t i = prompt.content.find(" ");
            std::string arg1 = prompt.content.substr(0, i);

            if (arg1 == "/model") {
                if (i == std::string::npos) {
                    std::cerr << "Usage: /model [name]\n";
                    continue;
                }
                std::string arg2 = prompt.content.substr(i+1);
                values->account->set_model(arg2);
                continue;
            }
            
            std::cerr << "Unexpected command.\n";
            continue;
        }

        values->conversation.push_back({"user", prompt.content, "", {}});

        std::cout << YELLOW << "Model: " << END;
        output = values->account->send_request(values->conversation);

        if (!output.tool_calls.empty()) {
            values->conversation.push_back({"assistant", "", "", output.tool_calls});

            handle_tool_calls(output, values->conversation);

            output.tool_calls.clear();
            output = values->account->send_request(values->conversation);
        }

        if (output.is_failed) {
            std::cout << "Request failed: " << output.content << "\n";
            values->conversation.pop_back();
            continue;
        }
        
        values->conversation.push_back({"assistant", output.content, "", {}});
        std::cout << "\n\n";

        if (values->limit > 0) {
            while (values->conversation.size() > values->limit) {
                values->conversation.erase(values->conversation.begin() + 1);
            }
        }

        save_chat(values->chats_path, values->conversation);
    }
}
