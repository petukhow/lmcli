#include <iostream>
#include <string>
#include "../include/json.hpp"
#include <curl/curl.h>
#include <vector>
#include <fstream>
#include <sstream>
#include "../include/message.h"

using json = nlohmann::json;

json loadConfig(const std::string& filepath) {
    std::ifstream file(filepath);
    std::string str;

    if (file) {
        std::ostringstream ss;
        ss << file.rdbuf(); // Read file buffer into the stringstream
        str = ss.str();     // Convert stringstream to std::string
    } else {
        // Handle file opening error
        std::cerr << "Error: Could not open the file." << std::endl;
    }
    json parsed = json::parse(str);
    return parsed;
}

bool limit_exceeded(const std::vector<Message>& conversation, size_t limit) {
    return conversation.size() >= limit;
}

static size_t writeCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    std::string* response = static_cast<std::string*>(userdata);
    response->append(ptr, size * nmemb);

    return size * nmemb;
}

Message makeRequest(const json& config, const std::vector<Message>& userMessages) {
        json requestBody;
        CURL *curl;
        CURLcode result;
        std::string rawResponse;
        Message response;
        struct curl_slist* headers = NULL;
        const std::string x_api_key = "Authorization: Bearer " + config["api_key"].get<std::string>();

        curl = curl_easy_init();

        requestBody["model"] = config["model"];
        requestBody["max_tokens"] = 1024;
        requestBody["messages"] = json::array();
        for (size_t i = 0; i < userMessages.size(); i++) {
            requestBody["messages"].push_back({{"content", userMessages[i].content}, {"role", userMessages[i].role}});
        }

        std::string body = requestBody.dump();
        
        if (curl) {
            headers = curl_slist_append(headers, "Content-Type: application/json");
            // headers = curl_slist_append(headers, "anthropic-version: 2023-06-01");
            headers = curl_slist_append(headers, x_api_key.c_str());

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_URL, config["api_url"].get<std::string>().c_str());
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
        };
        try {
            json parsed = json::parse(rawResponse);
            response.content = parsed["choices"][0]["message"]["content"].get<std::string>();
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            response.content = "";
        }

        return response;
    }

int main() {
    std::vector<Message> conversation;
    Message prompt;
    Message answer;

    json config = loadConfig("../config.json");
    conversation.push_back({"system", config["system_prompt"].get<std::string>()});
    size_t limit_messages = config["limit"];

    while (true) {
        std::cout << "Prompt (or '/exit' to end the conversation): ";
        std::getline(std::cin, prompt.content);
        if (prompt.content == "/exit") break; 
        conversation.push_back({"user", prompt.content});

        answer = makeRequest(config, conversation);
        if (answer.content == "") {
            std::cout << "Something went wrong. Try again." << "\n";
            conversation.pop_back();
            continue;
        }
        conversation.push_back({"assistant", answer.content});

        std::cout << answer.content << "\n";

        if (limit_exceeded(conversation, limit_messages)) {
                conversation.erase(conversation.begin() + 1); // erases user's message
                conversation.erase(conversation.begin() + 1); // erases model's answer
        }
    }

    return 0;
}