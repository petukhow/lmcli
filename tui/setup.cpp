#include "commands/commands.h"
#include "loaders/accounts.h"
#include "constants.h"
#include "logging/logger.h"
#include "providers/providers.h"
#include "json.hpp"
#include <ftxui/screen/color.hpp>
#include <vector>

#include "ftxui/component/component.hpp"         
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/component_options.hpp"
#include <ftxui/component/screen_interactive.hpp>
#include "ftxui/component/event.hpp"
#include <ftxui/dom/elements.hpp>

using json = nlohmann::json;
using namespace ftxui;

void setup() {
    const json providers = load_providers(PROVIDERS_FILE);
    if (providers.empty()) return;

    auto screen = ftxui::ScreenInteractive::Fullscreen();

    std::vector<std::string> entries;

    for (const auto& provider : providers["providers"]) {
        entries.push_back(provider["name"].get<std::string>());
    }
    entries.push_back("Exit");

    int selected = 0;
    MenuOption option;
    option.on_enter = screen.ExitLoopClosure();
    auto menu = Menu(&entries, &selected, option);

    screen.Loop(menu);
    if (size_t(selected) == entries.size()-1) return;
    log(LogLevel::Info, "Provider selected.");

    const auto& provider = providers["providers"][selected];
    const std::string type = provider["type"].get<std::string>();
    const std::string default_url = provider["default_api_url"].get<std::string>();
    const std::string default_model = provider["default_model"].get<std::string>();
    const std::string default_name = provider["name"].get<std::string>();

    json new_account;
    std::string account_name;
    std::string api_key;   
    std::string user_model;

    auto input_acc = Input(&account_name, "default: " + default_name);
    auto input_api_key = Input(&api_key, "API key");
    auto input_user_model = Input(&user_model, "default: " + default_model);

    auto component = Container::Vertical({
        input_acc,
        input_api_key,
        input_user_model
    });

    json accounts = load_accounts(ACCOUNTS_FILE);
    std::string final_name;
    auto accounts_json = accounts["accounts"];
    bool confirmed = false;
    auto final_component = CatchEvent(component, [&](Event event) {
        if (event == Event::Escape) {
            screen.Exit();
            return true;
        }
        if (event == Event::Return) {
            if (api_key.empty()) return true;
            confirmed = true;
            screen.Exit();
            return true;
        }
        return false;
    });

    auto renderer = Renderer(final_component, [&] {
        final_name = account_name.empty() ? default_name : account_name;
        bool duplicate = false;
        for (const auto& acc : accounts_json) {
            if (acc["name"].get<std::string>() == final_name) {
                duplicate = true;
                break;
            }
        }
        return vbox({
            hbox(text(" Account name > "), input_acc->Render()),
            hbox(text(" API Key      > "), input_api_key->Render()),
            hbox(text(" Model        > "), input_user_model->Render()),
            duplicate ? text(" Account with this name already exists! ") | color(Color::Yellow) 
                    : text(""),
                api_key.empty() ? text(" API key cannot be empty!") | color(Color::Red) : text(""),
        }) | border;
    });

    screen.Loop(renderer);

    if (!confirmed) return;
    if (account_name.empty()) account_name = default_name;
    if (user_model.empty()) user_model = default_model;

    std::string base_name = account_name;
    int i = 1;
    while (true) {
        bool exists = false;
        for (const auto& acc : accounts_json) {
            if (acc["name"].get<std::string>() == account_name) {
                exists = true;
                break;
            }
        }
        if (!exists) break;
        account_name = base_name + "-" + std::to_string(i);
        i++;
    }

    new_account = {
        {"type", type},
        {"name", account_name},
        {"api_key", api_key},
        {"api_url", default_url},
        {"model", user_model}
    };

    accounts["accounts"].push_back(new_account);
    save_accounts(accounts);
    log(LogLevel::Info, "New account created");
    return;
}