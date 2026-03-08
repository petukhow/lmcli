#include "config.h"
#include "config_l.h"
#include "constants.h"
#include "json.hpp"
#include <iostream>
#include <string>
#include "utils.h"
#include "fstream"

using json = nlohmann::json;

void edit_system_prompt() {
    json config = load_config(CONFIG_FILE);
    std::string new_system_prompt;
    std::string user_answer;
    
    std::cout << "Your current system prompt: " << config["system_prompt"].get<std::string>();
    
    while (true) {
        std::cout << "Do you want to change it? (y/n) ";
        if (!std::getline(std::cin, user_answer)) break;

        if (user_answer.empty()) continue;

        if (std::tolower(user_answer[0]) == 'y') {
            std::cout << "Enter new system prompt: ";
            if (!std::getline(std::cin, new_system_prompt)) return;
            config["system_prompt"] = new_system_prompt;
            break;
        } else if (std::tolower(user_answer[0]) == 'n') return;
    }
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