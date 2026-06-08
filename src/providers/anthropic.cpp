#include "constants.h"
#include "json.hpp"
#include "loaders/tools.h"
#include "logging/logger.h"
#include "utils/http_utils.h"
#include "streaming.h"
#include "types/roles.h"
#include "types/tools.h"
#include "anthropic.h"
#include <curl/curl.h>
#include "loaders/tools.h"

using json = nlohmann::json;

Message Anthropic::send_request(const std::vector<Message>& conversation, std::function<void(const std::string&)> callback) const {
    const std::string x_api_key = "x-api-key: " + api_key;
    CurlSlist headers;
    json request_body;
    Message response;
    Curl curl;

    request_body["model"] = model;
    request_body["max_tokens"] = max_tokens;
    request_body["messages"] = json::array();
    request_body["system"] = system_prompt;
    request_body["stream"] = true;
    request_body["tools"] = load_tools(TOOLS_FILE)["tools"];

    for (const auto& msg : conversation) {
        if (msg.role == Role::System) {
            continue;
        }

        if (msg.role == Role::Tool) {
            request_body["messages"].push_back({
        {"role", "user"},
        {"content", json::array({
            {
                {"type", "tool_result"},
                {"tool_use_id", msg.tool_call_id},
                {"content", msg.content}
                }
                })}
            });
        } 
        else if (msg.role == Role::Assistant && !msg.tool_calls.empty()) {
            json content = json::array();
            for (const auto& tool : msg.tool_calls) {
                content.push_back({
                    {"type", "tool_use"},
                    {"id", tool.id},
                    {"name", tool.name},
                    {"input", json::parse(tool.arguments)}
                });
            }
            request_body["messages"].push_back({
                {"role", "assistant"},
                {"content", content}
            });
        }
        else {
            request_body["messages"].push_back({
            {"content", msg.content},
            {"role", msg.role}
            });
        }
    }

    std::string body = request_body.dump();
    log(LogLevel::Debug, "Request body: " + body);
    log(LogLevel::Info, "Sending request to Anthropic: " + api_url);
        
    headers.append("Content-Type: application/json");
    headers.append("anthropic-version: 2023-06-01");
    headers.append(x_api_key.c_str());

    log(LogLevel::Debug, "API key: " + x_api_key);

    auto context = perform_request(body, headers, curl, callback);

    response.content = std::move(context.full_content);
    response.is_failed = context.is_failed;
    response.tool_calls = std::move(context.tool_calls);

    std::string is_failed = response.is_failed ? "true" : "false";
    log(LogLevel::Debug, "Got content: " + response.content);
    log(LogLevel::Debug, "Got 'is-failed' bool: " + is_failed);
    log(LogLevel::Debug, "Tool_calls count: " + std::to_string(response.tool_calls.size()));

    
    return response;
}

std::optional<std::string> Anthropic::extract_delta(const nlohmann::json& json) const {
    std::string delta;

    if (json.contains("delta") && json["delta"].contains("text")) {
        delta = json["delta"]["text"];
    }

    return delta;
}

void Anthropic::extract_tool_call(const nlohmann::json& json, StreamContext* context) const {
    if (json.contains("type") && json["type"] == "content_block_start") {
        if (json["content_block"]["type"] == "tool_use") {
            context->pending_tool.id = json["content_block"]["id"];
            context->pending_tool.name = json["content_block"]["name"];
        }
    }

    if (json.contains("type") && json["type"] == "content_block_delta") {
        if (json["delta"].contains("partial_json")) {
            context->tool_buffer += json["delta"]["partial_json"].get<std::string>();
        }
    }

    if (json.contains("type") && json["type"] == "content_block_stop") {
        if (!context->tool_buffer.empty()) {
            context->pending_tool.arguments = context->tool_buffer;
            context->tool_calls.push_back(context->pending_tool);
            context->tool_buffer.clear();
            context->pending_tool = ToolInfo{};
        }
    }
}