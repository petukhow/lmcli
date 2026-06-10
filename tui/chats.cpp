#include "chats.h"
#include "constants.h"
#include "utils/utils.h"
#include "json.hpp"
#include "logging/logger.h"
#include <cmath>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "types/message.h"
#include "terminal.h"

#include "ftxui/component/component.hpp"         
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/component_options.hpp"
#include <ftxui/component/screen_interactive.hpp>
#include "ftxui/component/event.hpp"
#include <ftxui/dom/elements.hpp>

using json = nlohmann::json;
using namespace ftxui;

std::string setup_chat() {
    const std::string chats_dir = get_chats_dir();
    const std::string chat_path;
    const auto chats = store_chats(chats_dir);
    std::string chat_name;
    std::string full_chat_name;

    if (!std::filesystem::exists(chats_dir)) {
        std::cerr << "Chats directory doesn't exist. Try 'lmcli init'";
        log(LogLevel::Error, "Chats directory doesn't exist.");
        return "";
    }

    if (chats.empty()) full_chat_name = create_chat(chats_dir);
    
    auto screen = ScreenInteractive::Fullscreen();

    std::vector<std::string> entries;
    for (const auto& chat : chats) {
        entries.push_back(chat.path().stem().string());
    }   
    entries.push_back("New chat");
    entries.push_back("Exit");

    int selected = 0;
    MenuOption option;
    option.on_enter = screen.ExitLoopClosure();
    auto menu = Menu(&entries, &selected, option);

    screen.Loop(menu);

    if (size_t(selected) == entries.size()-1) {
        return "";
    } else if (size_t(selected) == entries.size()-2) {
        full_chat_name = create_chat(chats_dir);
    } else {
        full_chat_name = chats[selected].path().string();
    }

    return full_chat_name;
}

bool save_chat(const std::string& filePath, const std::vector<Message>& chat) {
    std::ofstream conversation;
    const json j = {{"conversation", chat}};
    
    try {
        conversation.exceptions(std::ofstream::failbit);
        conversation.open(filePath);
        conversation << j.dump(4);

        return true;
    } catch (const std::exception& e) {

        log(LogLevel::Error, e.what());
        return false;
    }
}

std::vector<std::filesystem::directory_entry> store_chats(const std::string& chats_dir) {
    std::vector<std::filesystem::directory_entry> chats;

    if (!std::filesystem::exists(chats_dir)) {
        std::cerr << "\nChats directory not found. Try 'lmcli init'.\n";
        return chats;
    }

    if (!std::filesystem::is_empty(chats_dir)) {
        for (const auto& file : std::filesystem::directory_iterator(chats_dir)) {
            chats.push_back(file);
        }
    }
    return chats;
}

void print_chats(const std::vector<std::filesystem::directory_entry>& chats) {
    if (!chats.empty()) {
        std::cout << "Chats: \n";
        for (size_t i = 0; i < chats.size(); ++i) {
            std::cout << "[" + std::to_string(i+1) + "] ";
            std::cout << chats[i].path().stem().string() << "\n";
        }
    } else {
        log(LogLevel::Error, "Print chats request with no chats added.");
    }
}

std::string create_chat(const std::string& chats_dir) {
    std::string chat_name; 
    std::string full_chat_name; 
    std::string chat_path; 

    auto screen = ScreenInteractive::Fullscreen();

    auto input = Input(&chat_name, "Chat name");
    auto chatname = input | CatchEvent([&](Event event) {
        if (event == Event::Return) {
            screen.Exit();
            return true;
        }
        return false;
    });

    auto component = Container::Vertical({
        chatname
    });

    auto renderer = Renderer(component, [&] {
        return vbox({
            separator(),
            hbox(text(" Chat name > "), chatname->Render()),
            separator(),
        }) |
        border;
    });

    screen.Loop(renderer);

    if (chat_name == "/exit") return "";

    if (!chat_name.empty() && chat_name[0] == '/') {
        log(LogLevel::Error,"Tried to create a chat with '/' in the beginning of the filename.");
    }

    full_chat_name = chats_dir + chat_name + ".json";

    size_t files_amount = std::distance(std::filesystem::directory_iterator(chats_dir),
    std::filesystem::directory_iterator{});
    if (chat_name == "") {
        full_chat_name = chats_dir + "chat #" + std::to_string(files_amount) + ".json";
    }
    chat_path = create_file_if_not_exists(full_chat_name, CHAT_DEFAULT);
    return chat_path;
}

std::string continue_chat(const std::string& chats_dir, const std::string& chat_name,
    const std::vector<std::filesystem::directory_entry>& chats) {
    const std::string full_chat_name = chats_dir + chat_name + ".json";

    try {
        size_t chat_index = std::stoi(chat_name);
        if (chat_index >= 1 && chat_index <= chats.size()) {
            return chats[chat_index-1].path().string();
        } else {
            std::cerr << "No chat with index " << chat_index << ".\n";
            log(LogLevel::Error, "No chat with given index found.");
        }

    } catch (const std::invalid_argument&) {
        if (std::filesystem::exists(full_chat_name)) {
            return full_chat_name;
    } else {
        std::cerr << "No chat with given name found. Try again.\n";
        log(LogLevel::Error, "No chat with given name found.");
        return "";
        }
    }

    clear_lines(chats.size() + 2);

    return "";
}