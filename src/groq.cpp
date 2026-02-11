#include "../include/groq.h"
#include "../include/json.hpp"
#include <curl/curl.h>

using json = nlohmann::json;

static size_t writeCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    std::string* response = static_cast<std::string*>(userdata);
    response->append(ptr, size * nmemb);

    return size * nmemb;
}

Message Groq::sendRequest(const std::vector<Message>& conversation) const {
        json requestBody;
        CURL *curl;
        CURLcode result;
        std::string rawResponse;
        Message response;
        struct curl_slist* headers = NULL;
        const std::string x_api_key = "Authorization: Bearer " + api_key;

        curl = curl_easy_init();

        requestBody["model"] = model;
        requestBody["max_tokens"] = 1024;
        requestBody["messages"] = json::array();
        for (size_t i = 0; i < conversation.size(); i++) {
            requestBody["messages"].push_back({{"content", conversation[i].content}, {"role", conversation[i].role}});
        }

        std::string body = requestBody.dump();
        
        if (curl) {
            headers = curl_slist_append(headers, "Content-Type: application/json");
            // headers = curl_slist_append(headers, "anthropic-version: 2023-06-01");
            headers = curl_slist_append(headers, x_api_key.c_str());

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_URL, api_url.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
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
            response.content = parsed["choices"][0]["message"]["content"].get<std::string>();
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            response.content = "";
        }

        return response;
    }