#include "json.hpp"
#include "http_utils.h"
#include "google.h"
#include <curl/curl.h>

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