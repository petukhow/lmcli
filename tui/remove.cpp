#include "chats.h"
#include "commands.h"
#include "logging/logger.h"
#include "utils/utils.h"
#include <filesystem>
#include <vector>
#include <string>
#include "constants.h"
#include "loaders/accounts.h"

#include "ftxui/component/component.hpp"         
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/component_options.hpp"
#include <ftxui/component/screen_interactive.hpp>
#include "ftxui/component/event.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/screen.hpp>
#include "render.h"

using namespace ftxui;

void remove(const std::string& subcommand) {
    const std::string chats_dir = get_chats_dir();
    const auto chats = store_chats(chats_dir);

    auto screen = ScreenInteractive::Fullscreen();
    std::vector<std::string> entries = {
        "Chat",
        "Chats",
        "Account",
        "Exit"
    };

    int selected = 0;
    MenuOption option;
    option.on_enter = screen.ExitLoopClosure();
    auto menu = Menu(&entries, &selected, option);

    screen.Loop(menu);
    if (entries[selected] == "Exit") return;

    if (entries[selected] == "Chat") {
        if (chats.empty()) {
            show_message("No chats to remove.");
            return;
        }

        auto screen = ScreenInteractive::Fullscreen();
        std::vector<std::string> entries;

        for (const auto& chat : chats) {
            entries.push_back(chat.path().stem().string());
        }
        entries.push_back("Exit");

        int selected = 0;
        MenuOption option;
        option.on_enter = screen.ExitLoopClosure();
        auto menu = Menu(&entries, &selected, option);

        screen.Loop(menu);
        if (entries[selected] == "Exit") return;

        std::filesystem::remove(chats[selected].path());
        show_message("Removed successfully", Color::Green);
    }

    if (entries[selected] == "Chats") {
        if (chats.empty()) {
            show_message("No chats to remove.");
            return;
        }
        std::vector<std::string> confirm_entries = {"Yes", "No"};
        int confirm_selected = 0;
        auto confirm_screen = ScreenInteractive::Fullscreen();
        MenuOption confirm_option;
        confirm_option.on_enter = confirm_screen.ExitLoopClosure();
        auto confirm_menu = Menu(&confirm_entries, &confirm_selected, confirm_option);

        auto renderer = Renderer(confirm_menu, [&] {
            return vbox({
                text("WARNING: This will remove all chats.") | color(Color::Red),
                separator(),
                confirm_menu->Render(),
            }) | border;
        });

        confirm_screen.Loop(renderer);

        if (confirm_entries[confirm_selected] != "Yes") return;

        for (const auto& chat : std::filesystem::directory_iterator(chats_dir)) {
            std::filesystem::remove(chat);
        }
        log(LogLevel::Info, "All chats removed successfully");
        show_message("All chats removed successfully.", Color::Green);
    }

    if (entries[selected] == "Account") {
        nlohmann::json accounts = load_accounts(ACCOUNTS_FILE);
        const auto accounts_json = accounts["accounts"];
        auto screen = ScreenInteractive::Fullscreen();
        std::vector<std::string> entries;

        if (accounts["accounts"].empty()) {
            log(LogLevel::Error, "No accounts to remove.");
            show_message("No accounts to remove.");
            return;
        }

        for (const auto& account : accounts_json) {
            entries.push_back(account["name"].get<std::string>());
        }
        entries.push_back("Exit");

        int selected = 0;
        MenuOption option;
        option.on_enter = screen.ExitLoopClosure();
        auto menu = Menu(&entries, &selected, option);

        screen.Loop(menu);
        if (entries[selected] == "Exit") return;
        for (size_t i = 0; i < accounts["accounts"].size(); ++i) {
            if (accounts["accounts"][i]["name"].get<std::string>() == entries[selected]) {
                accounts["accounts"].erase(i);
                save_accounts(accounts);
                log(LogLevel::Info, "Account " + entries[selected] + " removed.");
                log(LogLevel::Info, "Accounts saved");
                show_message("Removed successfully!", Color::Green);
                return;
            }
        }
    }
    return;
}