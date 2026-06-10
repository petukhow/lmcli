#include "select_account.h"
#include "providers/provider.h"
#include "json.hpp"
#include <iostream>
#include "logging/logger.h"
#include <vector>

#include "ftxui/component/component.hpp"         
#include "ftxui/component/component_options.hpp"
#include <ftxui/component/screen_interactive.hpp>

using json = nlohmann::json;
using namespace ftxui; 

std::unique_ptr<Provider> select_account(const json& accounts, const json& config) {
    std::unique_ptr<Provider> provider;

    if (!accounts.contains("accounts") || accounts["accounts"].empty()) {
        log(LogLevel::Error, "Broken accounts.json config");
        return provider;
    }

    auto accounts_json = accounts["accounts"];

    if (accounts_json.size() == 1) {
        provider = Provider::create(accounts_json[0], config);
    } else {
        auto screen = ScreenInteractive::Fullscreen();
        std::vector<std::string> entries;
        for (const auto& account : accounts_json) {
            entries.push_back(account["name"].get<std::string>());
        }   
        entries.push_back("Exit");

        int selected = 0;
        MenuOption option;
        option.on_enter = screen.ExitLoopClosure();
        auto menu = Menu(&entries, &selected, option);

        screen.Loop(menu);
        if (size_t(selected) == entries.size()-1) return provider;

        provider = Provider::create(accounts_json[selected], config);
        log(LogLevel::Info, "Account selected: " + accounts_json[selected]["name"].get<std::string>());
    }
    return provider;
}