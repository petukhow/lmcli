#include "json.hpp"
#include <iostream>
#include "http_utils.h"
#include "open_ai_compatible.h"
#include <curl/curl.h>
#include <optional>

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
