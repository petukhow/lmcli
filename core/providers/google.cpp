#include "constants.h"
#include "json.hpp"
#include "loaders/tools.h"
#include "logging/logger.h"
#include "providers/streaming.h"
#include "types/roles.h"
#include "utils/http_utils.h"
#include "google.h"
#include "types/roles.h"
#include <curl/curl.h>
#include <optional>
#include <utility>

using json = nlohmann::json;

static nlohmann::json to_google_tools(const nlohmann::json& tools) {
    nlohmann::json result = nlohmann::json::array();
    for (const auto& tool : tools) {
        result.push_back({
            {"name", tool["name"]},
            {"description", tool["description"]},
            {"parameters", tool["input_schema"]},
        });
    }
    return nlohmann::json::array({{
    {"function_declarations", result}}
    });
}

static std::optional<std::string> find_tool_name(const std::vector<Message>& conversation, const std::string& tool_call_id) {
    std::string name;

    for (size_t i = 0; i < conversation.size(); ++i) {
        if (conversation[i].role == Role::Assistant) {
            for (size_t j = 0; j < conversation[i].tool_calls.size(); ++j) {
                if (conversation[i].tool_calls[j].id == tool_call_id) {
                    name = conversation[i].tool_calls[j].name;
                    return name;
                }
            }
        }
    }
    return std::nullopt;
}

Message Google::send_request(const std::vector<Message>& conversation, std::function<void(const std::string&)> callback) const {
    const std::string x_api_key = "x-goog-api-key: " + api_key;
    CurlSlist headers;
    json request_body;
    Message response;
    Curl curl;

    request_body["contents"] = json::array();
    request_body["systemInstruction"]["parts"] = json::array({json{{"text", system_prompt}}});
    request_body["generationConfig"]["maxOutputTokens"] = max_tokens;
    request_body["tools"] = to_google_tools(load_tools(TOOLS_FILE)["tools"]);

    for (const auto& msg : conversation) {
        if (msg.role == Role::System) continue;

        if (!msg.tool_calls.empty()) {
            json parts = json::array();
            for (const auto& tool : msg.tool_calls) {
                json part = {
                    {"functionCall", {
                        {"name", tool.name},
                        {"args", json::parse(tool.arguments)}
                    }}
                };
                if (tool.thought_signature.has_value()) {
                    part["thoughtSignature"] = *tool.thought_signature;
                }
                parts.push_back(part);
            }
            request_body["contents"].push_back({{"role", "model"}, {"parts", parts}});

        } else if (msg.role == Role::Tool) {
            auto name = find_tool_name(conversation, msg.tool_call_id);
            if (!name) continue;
            request_body["contents"].push_back({
                {"role", "user"},
                {"parts", json::array({{
                    {"functionResponse", {
                        {"name", *name},
                        {"response", {{"output", msg.content}}}
                    }}
                }})}
            });
        } else {
            std::string google_role = (msg.role == Role::Assistant) ? "model" : "user";
            request_body["contents"].push_back({
                {"role", google_role},
                {"parts", json::array({json{{"text", msg.content}}})}
            });
        }
    }
    std::string body = request_body.dump();

    log(LogLevel::Debug, "Request body: " + body);
    log(LogLevel::Info, "Sending request to Google: " + api_url);
        
    headers.append("Content-Type: application/json");
    headers.append(x_api_key.c_str());

    log(LogLevel::Debug, "API key: " + x_api_key);

    auto context = perform_request(body, headers, curl, callback);

    response.content = std::move(context.full_content);
    response.is_failed = context.is_failed;
    response.tool_calls = std::move(context.tool_calls);

    log(LogLevel::Debug, "Got content: " + response.content);
    log(LogLevel::Debug, "Got is_failed: " + std::string(response.is_failed ? "true" : "false"));
    log(LogLevel::Debug, "Tool_calls count: " + std::to_string(response.tool_calls.size()));
    
    return response;
}

std::optional<std::string> Google::extract_delta(const nlohmann::json& json) const {
    if (!json.contains("candidates") || !json["candidates"].is_array() || json["candidates"].empty()) return std::nullopt;

    auto candidates = json["candidates"][0];
    if (!candidates.contains("content") || candidates["content"].empty()) return std::nullopt;

    auto content = candidates["content"];
    if (!content.contains("parts") || !content["parts"].is_array() || content["parts"].empty()) return std::nullopt;

    auto parts = content["parts"][0];
    if (!parts.contains("text") || parts["text"].is_null()) return std::nullopt;

    return parts["text"];
}

void Google::extract_tool_call(const nlohmann::json& json, StreamContext* context) const {
    if (!json.contains("candidates") || json["candidates"].empty()) return;
    
    auto& candidate = json["candidates"][0];
    
    if (!candidate.contains("content") || !candidate["content"].contains("parts") 
        || candidate["content"]["parts"].empty()) return;
    
    auto& parts = candidate["content"]["parts"][0];
    
    if (parts.contains("functionCall")) {
        auto& tool = parts["functionCall"];
        if (tool.contains("name")) context->pending_tool.name = tool["name"];
        if (tool.contains("args")) context->pending_tool.arguments = tool["args"].dump();
    }

    if (parts.contains("thoughtSignature")) {
        context->pending_tool.thought_signature = parts["thoughtSignature"];
    }

    if (candidate.contains("finishReason") && !context->pending_tool.name.empty()) {
        context->tool_calls.push_back(context->pending_tool);
        context->pending_tool = ToolInfo{};
    }
}

