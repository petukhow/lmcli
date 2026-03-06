#include "json.hpp"
#include <iostream>
#include "http_utils.h"
#include "open_ai_compatible.h"
#include "streaming.h"
#include <curl/curl.h>

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

    for (const auto& msg : conversation) {
        request_body["messages"].push_back({
            {"content", msg.content},
            {"role", msg.role}
        });
    }

    std::string body = request_body.dump();

    headers.append("Content-Type: application/json");
    headers.append(x_api_key.c_str());

    auto [content, is_failed] = perform_request(body, headers, curl);

    response.content = content;
    response.is_failed = is_failed;
    return response;
}

void OpenAICompatible::event_handler(StreamContext* context) const {
    size_t event_end;
    while ((event_end = context->buffer.find("\n\n")) != std::string::npos) {
        std::string delta;
        std::string response;

        std::string event = context->buffer.substr(0, event_end);
        context->buffer.erase(0, event_end + 2);

        size_t data_index = event.find("data: ");
        if (data_index == std::string::npos) continue;
        response = event.substr(data_index + 6);

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
                context->full_content = delta;
                context->is_failed = true;
                break;
            }
        } catch (const std::exception& e) {
            std::cerr << "Broken response's json.\n";
        }

        std::cout << delta;
        std::cout.flush();
        context->full_content += delta;
    }
}
