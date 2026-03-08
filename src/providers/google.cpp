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
        request_body["contents"].push_back({
            {"parts", json::array({json{{"text", msg.content}}})},
            {"role", google_role}
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

void Google::event_handler(StreamContext* context) const {
    size_t event_end;
    while ((event_end = context->buffer.find("\n\n")) != std::string::npos) {
        std::string delta;
        std::string response;

        std::string event = context->buffer.substr(0, event_end);
        context->buffer.erase(0, event_end + 2);

        size_t data_index = event.find("data: ");
        if (data_index == std::string::npos) continue;
        response = event.substr(data_index + 6);

        try {
            nlohmann::json parsed = nlohmann::json::parse(response);
            
            if (parsed.contains("candidates") && parsed["candidates"][0].contains("content")
                && parsed["candidates"][0]["content"].contains("parts")
                && parsed["candidates"][0]["content"]["parts"][0].contains("text")) {
                delta = parsed["candidates"][0]["content"]["parts"][0]["text"];
            }

            if (parsed.contains("candidates") && parsed["candidates"][0].contains("finishReason")) break;

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