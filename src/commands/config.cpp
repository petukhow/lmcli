#include "config.h"
#include "config_l.h"
#include "constants.h"
#include "json.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include "utils.h"
#include "fstream"

using json = nlohmann::json;

void edit_system_prompt() {
    json config = load_config(CONFIG_FILE);
    std::string new_system_prompt;
    std::string user_answer;
    
    std::cout << "Your current system prompt: " << config["system_prompt"].get<std::string>() << "\n";
    
    while (true) {
        std::cout << "Do you want to change it? (y/n) ";
        if (!std::getline(std::cin, user_answer)) break;

        if (user_answer.empty()) continue;

        if (std::tolower(user_answer[0]) == 'y') {
            std::cout << "Enter new system prompt: ";
            if (!std::getline(std::cin, new_system_prompt)) return;
            
            break;
        } else if (std::tolower(user_answer[0]) == 'n') return;
    }

    config["system_prompt"] = new_system_prompt;
    save_config(config);
}

void edit_limit() {
    json config = load_config(CONFIG_FILE);
    std::string user_input;
    size_t new_limit_value;
    int raw_value;

    std::cout << "Your current limit: " << config["limit"].get<size_t>() << "\n";
    
    
    while (true) {
        try {
            std::cout << "Enter new limit value (enter to keep): ";
            if (!std::getline(std::cin, user_input)) return;
            if (user_input == "") return;
            raw_value = std::stoi(user_input);
            if (raw_value < 0) {
                std::cerr << "Limit value couldn't be negative.\n";
                continue;
            } 
            new_limit_value = static_cast<size_t>(raw_value);
            break;
        } catch (const std::invalid_argument&) {
            std::cerr << "Invalid argument.\n";
            continue;
        }
    }

    config["limit"] = new_limit_value;
    save_config(config);
}

void edit_max_tokens() {
    json config = load_config(CONFIG_FILE);
    std::string user_input;
    size_t new_limit_value;

    std::cout << "Your current limit: " << config["limit"].get<size_t>() << "\n";
    
    
    while (true) {
        try {
            std::cout << "Enter new limit value (enter to keep): ";
            if (!std::getline(std::cin, user_input)) return;
            if (user_input == "") return;
            int raw_value = std::stoi(user_input);
            if (raw_value < 0) {
                std::cerr << "Limit value couldn't be negative.\n";
                continue;
            } 
            new_limit_value = static_cast<size_t>(raw_value);
            break;
        } catch (const std::invalid_argument&) {
            std::cerr << "Invalid argument.\n";
            continue;
        }
    }

    config["limit"] = new_limit_value;
    save_config(config);
}

void save_config(const json& config) {
    std::string full_path = get_config_path(CONFIG_FILE);
    std::ofstream cfg;

    try {
        cfg.exceptions(std::ofstream::failbit);
        cfg.open(full_path);
        cfg << config.dump(4);
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }
}

int get_positive_number() {
    
}