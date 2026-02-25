#include "json.hpp"
#include <iostream>
#include "httpUtils.h"
#include "openAICompatible.h"
#include "streaming.h"
#include <curl/curl.h>

using json = nlohmann::json;

Message OpenAICompatible::sendRequest(const std::vector<Message>& conversation) const {
    const std::string x_api_key = "Authorization: Bearer " + api_key;
    CurlSlist headers;
    std::string rawResponse;
    json requestBody;
    Message response;
    Curl curl;

    requestBody["model"] = model;
    requestBody["max_tokens"] = max_tokens;
    requestBody["messages"] = json::array();

    for (const auto& msg : conversation) {
        requestBody["messages"].push_back({
            {"content", msg.content},
            {"role", msg.role}
        });
    }

    std::string body = requestBody.dump();

    headers.append("Content-Type: application/json");
    headers.append(x_api_key.c_str());

    std::string content = performRequest(body, headers, curl);

    try {
        json parsed = json::parse(content);
        if (parsed.contains("error")) {
            content = parsed["error"]["message"];
            response.isFailed = true;
        } else {
            content = parsed["choices"][0]["message"]["content"].get<std::string>();
        }
    } catch (const std::exception& e) {
        content = "unexpected response from provider.";
        response.isFailed = true;
    }

    return response;
}

void OpenAICompatible::eventHandler(StreamContext* context) const {
    size_t eventEnd;
    while ((eventEnd = context->buffer.find("\n\n")) != std::string::npos) {
        std::string delta;
        std::string response;

        std::string event = context->buffer.substr(0, eventEnd); 
        context->buffer.erase(0, eventEnd + 2);

        size_t dataIndex = event.find("data: ");
        if (dataIndex == std::string::npos) continue;
        response = event.substr(dataIndex + 6);

        if (response == "[DONE]") break;
        try {
            nlohmann::json parsed = nlohmann::json::parse(response);
            if (parsed.contains("choices") 
                && parsed["choices"][0].contains("delta")
                && parsed["choices"][0]["delta"].contains("content"))
            {
                delta = parsed["choices"][0]["delta"]["content"];
            }
            if (parsed.contains("error")) {
                delta = parsed["error"]["message"];
                context->fullContent = delta;
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