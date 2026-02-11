#include <iostream>
#include <string>
#include "../include/json.hpp"
#include <curl/curl.h>
#include <vector>
#include <fstream>
#include <sstream>
#include "../include/message.h"
#include "../include/groq.h"

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

int main() {
    std::vector<Message> conversation;
    Message prompt;
    Message answer;

    json config = loadConfig("../config.json");
    Groq groq = Groq(config["api_key"].get<std::string>(), config["api_url"].get<std::string>(),
     config["model"].get<std::string>(), config["system_prompt"].get<std::string>(), config["limit"]);
    conversation.push_back({"system", config["system_prompt"].get<std::string>()});
    size_t limit_messages = config["limit"];

    while (true) {
        std::cout << "Prompt (or '/exit' to end the conversation): ";
        std::getline(std::cin, prompt.content);
        if (prompt.content == "/exit") break; 
        conversation.push_back({"user", prompt.content});

        answer = groq.sendRequest(conversation);
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