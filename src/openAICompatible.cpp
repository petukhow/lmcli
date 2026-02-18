#include "json.hpp"
#include "httpUtils.h"
#include "openAICompatible.h"
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

    Provider::performRequest(body, headers, curl, rawResponse);

    try {
        json parsed = json::parse(rawResponse);
        if (parsed.contains("error")) {
            response.content = parsed["error"]["message"];
            response.isFailed = true;
        } else {
            response.content = parsed["choices"][0]["message"]["content"].get<std::string>();
        }
    } catch (const std::exception& e) {
        response.content = "";
        response.isFailed = true;
    }

    return response;
    }