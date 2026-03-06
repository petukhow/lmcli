#include "chats.h"
#include "utils.h"
#include "json.hpp"
#include <iostream>
#include <fstream>
#include "message.h"

using json = nlohmann::json;

std::string setup_chat() {
    std::string chat_path; // path to the chat file
    std::string chat_name; // chat name with extension
    std::string full_chat_name; // full path to the chat file (chats_dir + chat_name)
    std::string chats_dir = get_chats_dir(); // path to the chats directory

    print_chats(chats_dir);

    std::cout << "\nEnter chat's name to continue (/exit to leave, /new to create): \n";
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, chat_name);

        if (chat_name == "/exit") {
            break;
        } else if (chat_name == "/new") {
            full_chat_name = create_chat(chats_dir);
            break;
        } else {
            full_chat_name = continue_chat(chats_dir, chat_name);
            if (full_chat_name != "") {
                break;
            }
        }
    }
    return full_chat_name;
}

void save_chat(const std::string& filePath, const std::vector<Message>& chat) {;
    std::ofstream conversation;
    json j = {{"conversation", chat}};
    try {
        conversation.exceptions(std::ofstream::failbit);
        conversation.open(filePath);
        conversation << j.dump(4);
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }
}

json load_chats(const std::string& filepath) {
    std::ifstream file(filepath);
    json parsed = {};

    try {
        parsed = json::parse(file);
    } catch (const json::parse_error& e) {
        std::cerr << "Parse error: " << e.what() << "\n";
        return {};
    }
    return parsed;
}

void print_chats(const std::string chats_dir) {
    bool empty = std::filesystem::is_empty(chats_dir);

    if (!empty) {
        std::cout << "\nChats: \n";
        for (const auto& file : std::filesystem::directory_iterator(chats_dir)) {
            std::cout << "-- ";
            std::cout << file.path().stem().string() << "\n";
        }
    } else {
        std::cerr << "No chats yet. Start a new chat to create one.\n\n";
    }
}

std::string create_chat(const std::string& chats_dir) {
    std::string chat_name; // chat name with extension
    std::string full_chat_name; // full path to the chat file (chats_dir + chat_name)
    std::string chat_path; // path to the chat file

    std::cout << "\nEnter chat's name (leave empty for default: 'chat #...'): \n";

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, chat_name);

        if (chat_name == "/exit") return "";

        if (!chat_name.empty() && chat_name[0] == '/') {
            std::cerr << "Chat name cannot start with '/'. Try again.\n\n";
            continue;
        } else break;
    }

    full_chat_name = chats_dir + chat_name + ".json";

    const char* chat_template = R"({
        "conversation": []
    })";

    size_t files_amount = std::distance(std::filesystem::directory_iterator(chats_dir),
    std::filesystem::directory_iterator{});
    if (chat_name == "") {
        full_chat_name = chats_dir + "chat #" + std::to_string(files_amount) + ".json";
    }
    chat_path = create_file_if_not_exists(full_chat_name, chat_template);
    return chat_path;
}

std::string continue_chat(const std::string& chats_dir, const std::string& chat_name) {
    std::string full_chat_name = chats_dir + chat_name + ".json";
    if (std::filesystem::exists(full_chat_name)) {
        return full_chat_name;
    } else {
        std::cerr << "No chat with given name found. Try again.\n";
        return "";
    }
}
