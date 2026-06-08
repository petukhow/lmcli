#include "message.h"
#include "json.hpp"
#include "logging/logger.h"

void handle_tool_calls(const Message& output, std::vector<Message>& conversation) {
    for (const auto& tool : output.tool_calls) {
        nlohmann::json args;
        try {
            args = nlohmann::json::parse(tool.arguments);
        } catch (const nlohmann::json::parse_error& e) {
            conversation.push_back({Role::Tool, "Invalid tool args", tool.id, {}});
            log(LogLevel::Error, "Invalid tool args given");
            log(LogLevel::Error, e.what());
            log(LogLevel::Debug, "args: " + tool.arguments);
            continue;
        }

        if (tool.name == "exec_bash") {
            const std::string cmd = args["command"];
            log(LogLevel::Debug, "exec_bash args: " + cmd);

            std::string result = exec_bash(cmd);
            log(LogLevel::Debug, "exec_bash result: " + result);

            conversation.push_back({Role::Tool, result, tool.id, {}});
        }
        else {
            conversation.push_back({Role::Tool, "Unknown tool. You can use bash for writing/reading if you need.", tool.id, {}});
            log(LogLevel::Error, "Model called unknown tool.");
        }
    }
}