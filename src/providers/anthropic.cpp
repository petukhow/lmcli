#include "json.hpp"
#include "httpUtils.h"
#include "anthropic.h"
#include <curl/curl.h>

using json = nlohmann::json;

Message Anthropic::sendRequest(const std::vector<Message>& conversation) const {
    const std::string x_api_key = "x-api-key: " + api_key;
    CurlSlist headers;
    std::string rawResponse;
    json requestBody;
    Message response;
    Curl curl;

    requestBody["model"] = model;
    requestBody["max_tokens"] = max_tokens;
    requestBody["messages"] = json::array();
    requestBody["system"] = system_prompt;

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

    performRequest(body, headers, curl, rawResponse);

    try {
        json parsed = json::parse(rawResponse);
        if (parsed.contains("error")) {
            response.content = parsed["error"]["message"];
            response.isFailed = true;
        } else {
            response.content = parsed["content"][0]["text"].get<std::string>();
        }
    } catch (const std::exception& e) {
        response.content = "";
        response.isFailed = true;
    }

    return response;
}