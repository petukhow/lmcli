#include "json.hpp"
#include "http_utils.h"
#include "google.h"
#include <curl/curl.h>

using json = nlohmann::json;

Message Google::send_request(const std::vector<Message>& conversation) const {
    const std::string x_api_key = "x-goog-api-key: " + api_key;
    CurlSlist headers;
    std::string raw_response;
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
        std::string google_role = (msg.role == "assistant") ? "model" : msg.role;        
        request_body["contents"].push_back({
            {"parts", json::array({json{{"text", msg.content}}})},
            {"role", google_role}
        });
    }
    std::string body = request_body.dump();
        
    headers.append("Content-Type: application/json");
    headers.append(x_api_key.c_str());

    perform_request(body, headers, curl);

    try {
        json parsed = json::parse(raw_response);
        if (parsed.contains("error")) {
            response.content = parsed["error"]["message"];
            response.is_failed = true;
        } else {
            response.content = parsed["candidates"][0]["content"]["parts"][0]["text"].get<std::string>();
        }
    } catch (const std::exception& e) {
        response.content = "unexpected response from provider.";
        response.is_failed = true;
    }

    return response;
}

void Google::event_handler(StreamContext* context) const {}