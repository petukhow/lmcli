#include "config.h"
#include "constants.h"
#include "commands.h"
#include "json.hpp"
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

using json = nlohmann::json;

static std::optional<size_t> get_positive_number(const std::string& field_name) {
    int raw_value;
    std::string user_input;
    size_t new_value;

    while (true) {
        try {
            std::cout << "Enter new " << field_name << " value (enter to keep): ";
            if (!std::getline(std::cin, user_input)) return std::nullopt;
            if (user_input == "") return std::nullopt;
            raw_value = std::stoi(user_input);
            if (raw_value < 0) {
                std::cerr << field_name << " value couldn't be negative.\n";
                continue;
            } 
            new_value = static_cast<size_t>(raw_value);
            break;
        } catch (const std::invalid_argument&) {
            std::cerr << "Invalid argument.\n";
            continue;
        }
    }
    return new_value;
}

void edit_system_prompt() {
    json config = load_config(CONFIG_FILE);
    std::string new_system_prompt;
    std::string user_answer;
    
    std::cout << "Current: " << config["system_prompt"].get<std::string>() << "\n";
    
    while (true) {
        std::cout << "Do you want to change it? (y/n) ";
        if (!std::getline(std::cin, user_answer)) break;

        if (user_answer.empty()) continue;

        if (std::tolower(user_answer[0]) == 'y') {
            std::cout << "Enter new system prompt: ";
            if (!std::getline(std::cin, new_system_prompt)) return;

            config["system_prompt"] = new_system_prompt;
            save_config();
            break;
        } else {
            if (std::tolower(user_answer[0]) == 'n') {
                return;
            }
        }
    }
}

void edit_limit() {
    json config = load_config(CONFIG_FILE);

    std::cout << "Current: " << config["limit"].get<size_t>() << "\n";
    std::optional<size_t> new_limit_value = get_positive_number("limit");

    if (new_limit_value.has_value()) {
        config["limit"] = new_limit_value;
        save_config();
    }
}

void edit_max_tokens() {
    json config = load_config(CONFIG_FILE);

    std::cout << "Current: " << config["max_tokens"].get<size_t>() << "\n";
    std::optional<size_t> new_tokens_value = get_positive_number("max_tokens");

    if (new_tokens_value.has_value()) {
        config["max_tokens"] = new_tokens_value;
        save_config();
    }
}

void print_settings() {
    json config = load_config(CONFIG_FILE);

    std::cout << "Current settings:\n";
    std::cout << "  System prompt: " << config["system_prompt"].get<std::string>() << "\n";
    std::cout << "  Limit: " << config["limit"].get<size_t>() << "\n";
    std::cout << "  Max tokens: " << config["max_tokens"].get<size_t>() << "\n";
}

void config(const std::string& subcommand) {
    if (subcommand == "prompt") {
        edit_system_prompt();
    }
    else if (subcommand == "limit") {
        edit_limit();
    }
    else if (subcommand == "max-tokens") {
        edit_max_tokens();
    }
    else if (subcommand == "") {
        print_settings();
    }
    else std::cerr << "Usage: lmcli config [prompt|limit|max-tokens]\n";
}
