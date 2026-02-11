#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <curl/curl.h>
#include "../include/message.h"
#include "../include/groq.h"
#include "../include/json.hpp"

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

bool limitExceeded(const std::vector<Message>& conversation, size_t limit) {
    return conversation.size() >= limit;
}

int main() {
    json config = loadConfig("../config.json");
    std::vector<Message> conversation;
    std::unique_ptr<Provider> provider;
    std::string providerName;
    Message prompt;
    Message answer;
    bool isValidName = false; 

    while (true) {
        std::cout << "Pick a provider from the list below (/exit to leave):" << "\n";

        for (size_t i = 0; i < config["providers"]["openai-compatible"].size(); i++) {
            std::cout << config["providers"]["openai-compatible"][i]["name"].get<std::string>() << "\n";
        }   
        if (config["providers"].contains("Anthropic")) {
            std::cout << "Anthropic" << "\n";
        }

        std::cout << "> ";
        std::getline(std::cin, providerName);
        if (providerName == "/exit") break;

        isValidName = false;
        for (size_t i = 0; i < config["providers"]["openai-compatible"].size(); i++) {
            if (providerName == config["providers"]["openai-compatible"][i]["name"].get<std::string>()) {
                provider = std::make_unique<Groq>(config["providers"]["openai-compatible"][i]["api_key"].get<std::string>(),
         config["providers"]["openai-compatible"][i]["api_url"].get<std::string>(),
            config["providers"]["openai-compatible"][i]["model"].get<std::string>(),
     config["system_prompt"].get<std::string>(), config["limit"].get<size_t>());
                isValidName = true;
                break;
            }
        }
        // else if (provider_name == "Anthropic") {
        //     is_valid_name = true;
        // }
        if (isValidName) break;
    }
    conversation.push_back({"system", config["system_prompt"].get<std::string>()});
    size_t limit_messages = config["limit"].get<size_t>();

    while (true) {
        std::cout << "Prompt (or '/exit' to end the conversation): ";
        std::getline(std::cin, prompt.content);
        if (prompt.content == "/exit") break; 
        conversation.push_back({"user", prompt.content});

        answer = provider->sendRequest(conversation);
        if (answer.content == "") {
            std::cout << "Something went wrong. Try again." << "\n";
            conversation.pop_back();
            continue;
        }
        conversation.push_back({"assistant", answer.content});

        std::cout << answer.content << "\n";

        if (limitExceeded(conversation, limit_messages)) {
                conversation.erase(conversation.begin() + 1); // erases user's message
                conversation.erase(conversation.begin() + 1); // erases model's answer
        }
    }

    return 0;
}