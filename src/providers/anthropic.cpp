#include "json.hpp"
#include "httpUtils.h"
#include "anthropic.h"
#include <curl/curl.h>
#include <iostream>
#include "streaming.h"

using json = nlohmann::json;

Message Anthropic::sendRequest(const std::vector<Message>& conversation) const {
    const std::string x_api_key = "x-api-key: " + api_key;
    CurlSlist headers;
    json requestBody;
    Message response;
    Curl curl;

    requestBody["model"] = model;
    requestBody["max_tokens"] = max_tokens;
    requestBody["messages"] = json::array();
    requestBody["system"] = system_prompt;
    requestBody["stream"] = true;

    for (const auto& msg : conversation) {
        if (msg.role == "system") {
            continue;
        }
        requestBody["messages"].push_back({
            {"content", msg.content},
            {"role", msg.role}
        });
    }

    std::string body = requestBody.dump();
        
    headers.append("Content-Type: application/json");
    headers.append("anthropic-version: 2023-06-01");
    headers.append(x_api_key.c_str());

    auto [content, isFailed] = performRequest(body, headers, curl);

    response.content = content;
    response.isFailed = isFailed;
    return response;
}

void Anthropic::eventHandler(StreamContext* context) const {
    size_t eventEnd;
    while ((eventEnd = context->buffer.find("\n\n")) != std::string::npos) {
        std::string delta;
        std::string response;

        std::string event = context->buffer.substr(0, eventEnd); 
        context->buffer.erase(0, eventEnd + 2);

        size_t dataIndex = event.find("data: ");
        if (dataIndex == std::string::npos) continue;
        response = event.substr(dataIndex + 6);

        try {
            nlohmann::json parsed = nlohmann::json::parse(response);
            if (parsed.contains("type") && parsed["type"] == "message_stop") break;

            if (parsed.contains("delta") && parsed["delta"].contains("text")) {
                delta = parsed["delta"]["text"];
            }
            if (parsed.contains("error")) {
                delta = parsed["error"]["message"];
                context->fullContent = delta;
                context->isFailed = true;
                break;
            }
        } catch (const std::exception& e) {
            std::cerr << "Broken response's json.\n";  
        }

        std::cout << delta;
        std::cout.flush();
        context->fullContent += delta;
    }
}