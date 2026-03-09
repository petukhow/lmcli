#include "json.hpp"
#include <iostream>
#include "http_utils.h"
#include "streaming.h"
#include "tools.h"
#include "open_ai_compatible.h"
#include <curl/curl.h>
#include <optional>
#include <vector>

using json = nlohmann::json;

Message OpenAICompatible::send_request(const std::vector<Message>& conversation) const {
    const std::string x_api_key = "Authorization: Bearer " + api_key;
    CurlSlist headers;
    json request_body;
    Message response;
    Curl curl;

    request_body["model"] = model;
    request_body["max_tokens"] = max_tokens;
    request_body["messages"] = json::array();
    request_body["stream"] = true;
    request_body["tools"] = json::array({
    {
        {"type", "function"},
        {"function", {
            {"name", "read_file"},
            {"description", "Read the contents of a file at a given path"},
            {"parameters", {
                {"type", "object"},
                {"properties", {
                    {"file", {
                        {"type", "string"},
                        {"description", "Path to a file"}
                    }}
                }},
                {"required", json::array({"file"})},
                {"additionalProperties", false}
                }}
            }}
        }
    });

    for (const auto& msg : conversation) {
        json message = {{"role", msg.role}};
        
        if (msg.role == "tool") {
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

    std::string body = request_body.dump();

    headers.append("Content-Type: application/json");
    headers.append(x_api_key.c_str());

    auto context = perform_request(body, headers, curl);

    response.content = context.full_content;
    response.is_failed = context.is_failed;
    response.tool_calls = context.tool_calls;
    
    return response;
}

std::optional<std::string> OpenAICompatible::extract_delta(const nlohmann::json& json) const {
    std::string delta;

    try {
        if (json.contains("choices")
            && json["choices"][0].contains("delta")
            && json["choices"][0]["delta"].contains("content"))
        {
            delta = json["choices"][0]["delta"]["content"];
        }
    } catch (const std::exception& e) {
        std::cerr << "Broken response's json.\n";
        return std::nullopt;
    }

    return delta;
}

std::optional<ToolInfo> OpenAICompatible::extract_tool_call(const nlohmann::json& json) const {
    if (json["choices"][0]["delta"].contains("tool_calls")) {
        auto& tool = json["choices"][0]["delta"]["tool_calls"][0];
        ToolInfo tool_info;
        
        if (tool.contains("id")) tool_info.id = tool["id"];
        if (tool.contains("function")) {
            if (tool["function"].contains("name")) tool_info.name = tool["function"]["name"];
            if (tool["function"].contains("arguments")) tool_info.arguments = tool["function"]["arguments"];
        }
        return tool_info;
        }
    return std::nullopt;
}
