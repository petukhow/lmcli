#include "json.hpp"
#include "httpUtils.h"
#include "openAICompatible.h"
#include <curl/curl.h>
#include <iostream>

using json = nlohmann::json;

Message OpenAICompatible::sendRequest(const std::vector<Message>& conversation) const {
    const std::string x_api_key = "Authorization: Bearer " + api_key;
    CurlSlist headers;
    std::string rawResponse;
    json requestBody;
    Message response;
    CURLcode result;
    Curl curl;

    requestBody["model"] = model;
    requestBody["max_tokens"] = max_tokens;
    requestBody["messages"] = json::array();

    for (size_t i = 0; i < conversation.size(); i++) {
        requestBody["messages"].push_back({
            {"content", conversation[i].content},
            {"role", conversation[i].role}
        });
    }

    std::string body = requestBody.dump();

    headers.append("Content-Type: application/json");
    headers.append(x_api_key.c_str());

    curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers.get());
    curl_easy_setopt(curl.get(), CURLOPT_URL, api_url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &rawResponse);

    result = curl_easy_perform(curl.get());
    
    if (result != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed:\n";
        curl_easy_strerror(result);
    } 

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
    }

    return response;
    }