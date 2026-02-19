#include "json.hpp"
#include "httpUtils.h"
#include "google.h"
#include <curl/curl.h>

using json = nlohmann::json;

Message Google::sendRequest(const std::vector<Message>& conversation) const {
    const std::string x_api_key = "x-goog-api-key: " + api_key;
    CurlSlist headers;
    std::string rawResponse;
    json requestBody;
    Message response;
    Curl curl;

    requestBody["contents"] = json::array();
    requestBody["systemInstruction"]["parts"] = json::array({json{{"text", system_prompt}}});
    requestBody["generationConfig"]["maxOutputTokens"] = max_tokens;

    for (const auto& msg : conversation) {
        if (msg.role == "system") {
            continue;
        }
        requestBody["contents"].push_back({
            {"parts", json::array({json{{"text", msg.content}}})},
            {"role", msg.role}
        });
    }
    std::string body = requestBody.dump();
        
    headers.append("Content-Type: application/json");
    headers.append(x_api_key.c_str());

    performRequest(body, headers, curl, rawResponse);

    try {
        json parsed = json::parse(rawResponse);
        if (parsed.contains("error")) {
            response.content = parsed["error"]["message"];
            response.isFailed = true;
        } else {
            response.content = parsed["candidates"][0]["content"]["parts"][0]["text"].get<std::string>();
        }
    } catch (const std::exception& e) {
        response.content = "";
        response.isFailed = true;
    }

    return response;
}