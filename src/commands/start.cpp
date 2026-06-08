#include "commands.h"
#include "json.hpp"
#include "providers/provider.h"
#include "select_account.h"
#include "constants.h"
#include "chats.h"
#include "loaders/config.h"
#include "loaders/accounts.h"
#include "loaders/json_io.h"
#include "types/message.h"
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "types/roles.h"
#include "ansi_codes.h"
#include "types/handle_tool_calls.h"
#include "logging/logger.h"
#include "utils/utils.h"

using json = nlohmann::json;

using namespace ansi;

struct ChatValues {
    std::vector<Message> conversation;
    std::unique_ptr<Provider> account;
    std::string chats_path;
    size_t limit;
};

static std::optional<ChatValues> chat_init() {
    std::vector<Message> conversation;
    const json config = load_config(CONFIG_FILE);
    const json accounts = load_accounts(ACCOUNTS_FILE);

    if (config.is_null() || accounts.is_null()) {
        std::cerr << "Failed to load config or accounts. Try 'lmcli init'.\n";
        log(LogLevel::Error, "Failed to load config or accounts");
        return std::nullopt;
    }

    auto account = select_account(accounts, config);
    if (!account) {
        return std::nullopt;
    }

    const std::string chats_path = setup_chat();
    if (chats_path.empty()) return std::nullopt;

    const json chats = load_json(chats_path);

    if (chats.is_null()) {
        return std::nullopt;
    }

    if (chats.contains("conversation") && chats["conversation"].is_array()) {
        conversation = chats["conversation"].get<std::vector<Message>>();
    }

    if (!conversation.empty()) {
        if (conversation[0].content != config["system_prompt"].get<std::string>()) {
            conversation[0].content = config["system_prompt"];
        }
    }

    if (conversation.empty()) {
        conversation.push_back({Role::System, config["system_prompt"].get<std::string>(), "", {}});
    }

    return ChatValues{std::move(conversation), std::move(account), std::move(chats_path), config["limit"]};
}

void start() {
    Message prompt;
    Message output; 

    auto values = chat_init(); 
    if (!values) {
        log(LogLevel::Error, "Chat values not initialized");
        return;
    } 

    std::cout << "Enter /help for available commands.\n";
    std::cout << "Prompt (or '/exit' to end the conversation): \n";
    while (true) {
        const auto input = readline(CYAN + "You: " + END); // user's prompt
        if (!input) break;
        prompt.content = *input;
        if (prompt.content == "") continue;

        if (prompt.content[0] == '/') {
            if (prompt.content == "/exit") {
                log(LogLevel::Info, "Exited from conversation");
                break;
            } 
            if (prompt.content == "/help") {
                std::cout << "Available commands:\n";
                std::cout << "  /exit - End the conversation\n";
                std::cout << "  /model [name] - Switch to a different model\n";
                continue;
            }
            
            const size_t i = prompt.content.find(" ");
            const std::string arg1 = prompt.content.substr(0, i);

            if (arg1 == "/model") {
                if (i == std::string::npos) {
                    std::cerr << "Usage: /model [name]\n";
                    continue;
                }
                const std::string arg2 = prompt.content.substr(i+1);
                values->account->set_model(arg2);
                log(LogLevel::Info, "Model changed.");
                continue;
            }
            
            std::cerr << "Unknown command.\n";
            log(LogLevel::Error, "Unknown command");
            continue;
        }

        values->conversation.push_back({Role::User, prompt.content, "", {}});
        log(LogLevel::Debug, "User prompted: " + prompt.content);

        std::cout << YELLOW << "Model: " << END;
        output = values->account->send_request(values->conversation, [](const std::string& delta) {
            std::cout << delta;
            std::cout.flush();
        });
        log(LogLevel::Debug, "User prompted: " + prompt.content);

        while (!output.tool_calls.empty()) {
            values->conversation.push_back({Role::Assistant, "", "", output.tool_calls});
            handle_tool_calls(output, values->conversation);
            log(LogLevel::Debug, "Number of tool calls: " + std::to_string(output.tool_calls.size()));
            output.tool_calls.clear();
            output = values->account->send_request(values->conversation, [](const std::string& delta) {
                std::cout << delta;
                std::cout.flush();
            });

            const std::string is_failed = output.is_failed ? "true" : "false";
            log(LogLevel::Debug, "API returned output: " + output.content);
            log(LogLevel::Debug, "Is API request failed: " + is_failed);
            log(LogLevel::Debug, "Number of tool calls: " + std::to_string(output.tool_calls.size()));
        }
        
        if (output.is_failed) {
            std::cerr << "Request failed: " << output.content << "\n";
            log(LogLevel::Error, "Request failed with error: " + output.content);
            values->conversation.pop_back();
            continue;
        }
        
        values->conversation.push_back({Role::Assistant, output.content, "", {}});
        log(LogLevel::Debug, "Model's output (line 182, start.cpp): " + output.content);
        std::cout << "\n";

        if (values->limit > 0) {
            while (values->conversation.size() > values->limit) {
                values->conversation.erase(values->conversation.begin() + 1);
            }
        }

        save_chat(values->chats_path, values->conversation);
    }
}
