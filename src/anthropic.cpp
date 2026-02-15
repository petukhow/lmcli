#include "json.hpp"
#include "anthropic.h"
#include "config.h"
#include "utils.h"
#include <curl/curl.h>

using json = nlohmann::json;

Message Anthropic::sendRequest(const std::vector<Message>& conversation) const {
    const std::string x_api_key = "x-api-key: " + api_key;
    struct curl_slist* headers = NULL;
    std::string rawResponse;
    json requestBody;
    Message response;
    CURLcode result;
    CURL *curl;

    curl = curl_easy_init();

    requestBody["model"] = model;
    requestBody["max_tokens"] = max_tokens;
    requestBody["messages"] = json::array();
    requestBody["system"] = system_prompt;

    for (size_t i = 0; i < conversation.size(); i++) {
        if (conversation[i].role == "system") {
            continue;
        }
        requestBody["messages"].push_back({
            {"content", conversation[i].content},
            {"role", conversation[i].role}
        });
    }

    std::string body = requestBody.dump();
        
    if (curl) {
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "anthropic-version: 2023-06-01");
        headers = curl_slist_append(headers, x_api_key.c_str());

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, api_url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rawResponse);

        result = curl_easy_perform(curl);

        if (result != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(result));
        } 
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        response.content = "";
    }
    try {
        json parsed = json::parse(rawResponse);
        response.content = parsed["content"][0]["text"].get<std::string>();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        response.content = "";
    }

    return response;
    }