#include "json.hpp"
#include "http_utils.h"
#include "streaming.h"
#include "tools.h"
#include "anthropic.h"
#include <curl/curl.h>
#include <iostream>

using json = nlohmann::json;

Message Anthropic::send_request(const std::vector<Message>& conversation) const {
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
    request_body["tools"] = json::array({
    {
        {"name", "read_file"},
        {"description", "Read the contents of a file at a given path"},
        {"input_schema", {
            {"type", "object"},
            {"properties", {
                {"file", {
                    {"type", "string"},
                    {"description", "Path to a file"}
                }}
            }},
            {"required", json::array({"file"})}
            }}
        }
    });

    for (const auto& msg : conversation) {
        if (msg.role == "system") {
            continue;
        }

        if (msg.role == "tool") {
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
        else if (msg.role == "assistant" && !msg.tool_calls.empty()) {
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
        
    headers.append("Content-Type: application/json");
    headers.append("anthropic-version: 2023-06-01");
    headers.append(x_api_key.c_str());

    auto context = perform_request(body, headers, curl);

    response.content = context.full_content;
    response.is_failed = context.is_failed;
    response.tool_calls = context.tool_calls;
    
    return response;
}

std::optional<std::string> Anthropic::extract_delta(const nlohmann::json& json) const {
    std::string delta;
    try {
        if (json.contains("delta") && json["delta"].contains("text")) {
            delta = json["delta"]["text"];
        }
    } catch (const std::exception& e) {
        std::cerr << "Broken response's json.\n";  
        return std::nullopt;
    }

    return delta;
}

std::optional<ToolInfo> Anthropic::extract_tool_call(const nlohmann::json& json) const {
    return std::nullopt;
}