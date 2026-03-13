#include "json.hpp"
#include "http_utils.h"
#include "google.h"
#include <curl/curl.h>
#include <iostream>

using json = nlohmann::json;

Message Google::send_request(const std::vector<Message>& conversation) const {
    const std::string x_api_key = "x-goog-api-key: " + api_key;
    CurlSlist headers;
    json request_body;
    Message response;
    Curl curl;

    request_body["contents"] = json::array();
    request_body["systemInstruction"]["parts"] = json::array({json{{"text", system_prompt}}});
    request_body["generationConfig"]["maxOutputTokens"] = max_tokens;

    for (const auto& msg : conversation) {
        if (msg.role == "system") {
            continue;
        }
        
        std::string google_role;  
        if (msg.role == "assistant") {
            google_role = "model";
        }
        else if (msg.role == "user") {
            google_role = "user";
        }      

        request_body["contents"].push_back({
            {"parts", json::array({json{{"text", msg.content}}})},
            {"role", google_role}
        });
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

std::optional<std::string> Google::extract_delta(const nlohmann::json& json) const {
    std::string delta;
    try {  
        if (json.contains("candidates") && json["candidates"][0].contains("content")
            && json["candidates"][0]["content"].contains("parts")
            && json["candidates"][0]["content"]["parts"][0].contains("text")) {
            delta = json["candidates"][0]["content"]["parts"][0]["text"];
        }

    } catch (const std::exception& e) {
        std::cerr << "Broken response's json.\n";  
    }
    return delta;
}

void Google::extract_tool_call(const nlohmann::json& json, StreamContext* context) const {
    return;
}
