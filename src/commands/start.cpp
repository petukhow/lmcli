#include "commands.h"
#include "json.hpp"
#include "providers/provider.h"
#include "select_account.h"
#include "constants.h"
#include "chats.h"
#include "loaders/config.h"
#include "loaders/accounts.h"
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
#include "types/tools.h"

using json = nlohmann::json;

using namespace ansi;

struct ChatValues {
    std::vector<Message> conversation;
    std::unique_ptr<Provider> account;
    std::string chats_path;
    size_t limit;
};

static void handle_tool_calls(const Message& output, std::vector<Message>& conversation) {
    for (const auto& tool : output.tool_calls) {
        json args;
        try {
            args = nlohmann::json::parse(tool.arguments);
        } catch (const nlohmann::json::parse_error& e) {
            conversation.push_back({Role::Tool, "Invalid tool args", tool.id, {}});
            continue;
        }

        if (tool.name == "read_file") {
            std::string path = args["file"];

            std::string result = read_file(path);

            conversation.push_back({Role::Tool, result, tool.id, {}});
        }
        else if (tool.name == "exec_bash") {
            std::string cmd = args["command"];

            std::string result = exec_bash(cmd);

            conversation.push_back({Role::Tool, result, tool.id, {}});
        }
        else {
            conversation.push_back({Role::Tool, "Unknown tool. You can use bash for writing/reading if you need.", tool.id, {}});
        }
    }
}

static std::optional<ChatValues> chat_init() {
    std::vector<Message> conversation;
    json config = load_config(CONFIG_FILE);
    json accounts = load_accounts(ACCOUNTS_FILE);

    if (config.is_null() || accounts.is_null()) {
        std::cerr << "Failed to load config or accounts. Try 'lmcli init'.\n";
        return std::nullopt;
    }

    auto account = select_account(accounts, config);
    if (!account) return std::nullopt;

    std::string chats_path = setup_chat();
    if (chats_path.empty()) return std::nullopt;

    json chats = load_chats(chats_path);

    if (chats.is_null()) {
        std::cerr << "Failed to load chats. Try 'lmcli init'.\n";
        return std::nullopt;
    }

    if (chats.contains("conversation") && chats["conversation"].is_array()) {
        conversation = chats["conversation"].get<std::vector<Message>>();
    }

    if (!conversation.empty()) {
        if (conversation[0].content != config["system_prompt"]) {
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
    if (!values) return;

    std::cout << "Enter /help for available commands.\n";
    std::cout << "Prompt (or '/exit' to end the conversation): \n";
    while (true) {
        std::cout << CYAN << "You: " << END;
        if (!std::getline(std::cin, prompt.content)) break; // user's prompt
        if (prompt.content == "") continue;

        if (prompt.content[0] == '/') {
            if (prompt.content == "/exit") break; 
            if (prompt.content == "/help") {
                std::cout << "Available commands:\n";
                std::cout << "  /exit - End the conversation\n";
                std::cout << "  /model [name] - Switch to a different model\n";
                continue;
            }
            
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

        values->conversation.push_back({Role::User, prompt.content, "", {}});

        std::cout << YELLOW << "Model: " << END;
        output = values->account->send_request(values->conversation);

        while (!output.tool_calls.empty()) {
            values->conversation.push_back({Role::Assistant, "", "", output.tool_calls});
            handle_tool_calls(output, values->conversation);
            output.tool_calls.clear();
            output = values->account->send_request(values->conversation);
        }
        
        if (output.is_failed) {
            std::cout << "Request failed: " << output.content << "\n";
            values->conversation.pop_back();
            continue;
        }
        
        values->conversation.push_back({Role::Assistant, output.content, "", {}});
        std::cout << "\n\n";

        if (values->limit > 0) {
            while (values->conversation.size() > values->limit) {
                values->conversation.erase(values->conversation.begin() + 1);
            }
        }

        save_chat(values->chats_path, values->conversation);
    }
}
