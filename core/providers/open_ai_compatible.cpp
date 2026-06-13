#include "constants.h"
#include "json.hpp"
#include "loaders/tools.h"
#include "logging/logger.h"
#include "utils/http_utils.h"
#include "providers/streaming.h"
#include "types/tools.h"
#include "open_ai_compatible.h"
#include <curl/curl.h>
#include <optional>
#include <vector>
#include "types/roles.h"

using json = nlohmann::json;

static nlohmann::json to_openai_tools(const nlohmann::json& tools) {
    nlohmann::json result = nlohmann::json::array();
    for (const auto& tool : tools) {
        result.push_back({
            {"type", "function"},
            {"function", {
                {"name", tool["name"]},
                {"description", tool["description"]},
                {"parameters", tool["input_schema"]}
            }}
        });
    }
    return result;
}

Message OpenAICompatible::send_request(const std::vector<Message>& conversation, std::function<void(const std::string&)> callback,
    std::atomic<bool>* cancelled) const {
    const std::string x_api_key = "Authorization: Bearer " + api_key;
    CurlSlist headers;
    json request_body;
    std::string body;
    Message response;
    Curl curl;

    request_body["model"] = model;
    request_body["max_tokens"] = max_tokens;
    request_body["messages"] = json::array();
    request_body["stream"] = true;
    request_body["tools"] = to_openai_tools(load_tools(TOOLS_FILE)["tools"]);

    
    for (const auto& msg : conversation) {
        json message = {{"role", role_to_string(msg.role)}};
            
        if (msg.role == Role::Tool) {
            message["content"] = msg.content;
            message["tool_call_id"] = msg.tool_call_id;
        } else if (!msg.tool_calls.empty()) {
            message["tool_calls"] = json::array();
            for (const auto& tool : msg.tool_calls) {
                message["tool_calls"].push_back({
                    {"id", tool.id},
                    {"type", "function"},
                    {"function", {
                        {"name", tool.name},
                        {"arguments", tool.arguments}
                    }}
                });
            }
        } else {
            message["content"] = msg.content;
        }
        
        request_body["messages"].push_back(message);
    }

    body = request_body.dump();

    log(LogLevel::Debug, "Request body: " + body);
    log(LogLevel::Info, "Sending request to OpenAI: " + api_url);

    headers.append("Content-Type: application/json");
    headers.append(x_api_key.c_str());

    log(LogLevel::Debug, "API key: " + x_api_key);

    auto context = perform_request(body, headers, curl, callback, cancelled);

    response.content = context.full_content;
    response.is_failed = context.is_failed;
    response.tool_calls = context.tool_calls;

    log(LogLevel::Debug, "Got content: " + response.content);
    log(LogLevel::Debug, "Got is_failed: " + std::string(response.is_failed ? "true" : "false"));
    log(LogLevel::Debug, "Tool_calls count: " + std::to_string(response.tool_calls.size()));

    return response;
}

std::optional<std::string> OpenAICompatible::extract_delta(const nlohmann::json& json) const {
    if (!json.contains("choices") || !json["choices"].is_array() || json["choices"].empty()) return std::nullopt;

    auto choices = json["choices"][0];
    if (!choices.contains("delta") || choices["delta"].empty()) return std::nullopt;

    auto delta = choices["delta"];
    if (!delta.contains("content") || delta["content"].is_null()) return std::nullopt;

    return delta["content"];
}

void OpenAICompatible::extract_tool_call(const nlohmann::json& json, StreamContext* context) const {
    if (!json.contains("choices") || json["choices"].empty()) return;

    auto& choices = json["choices"][0];

    if (!choices.contains("delta") || choices["delta"].empty()) return;

    auto& delta = json["choices"][0]["delta"];

    if (delta.contains("tool_calls")) {
        auto& tool = delta["tool_calls"][0];
        
        if (tool.contains("id")) context->pending_tool.id = tool["id"];
        if (tool.contains("function")) {
            auto& function = tool["function"];
            if (function.contains("name")) context->pending_tool.name = function["name"];
            if (function.contains("arguments")) context->pending_tool.arguments += function["arguments"];
        }
    }

    if (choices.contains("finish_reason")) {
        if (choices["finish_reason"] == "tool_calls") {
            if (!context->pending_tool.arguments.empty()) {
                context->tool_calls.push_back(context->pending_tool);
                context->pending_tool = ToolInfo{};
            }
        }
    }
}
