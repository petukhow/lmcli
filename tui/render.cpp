#include "render.h"

#include "ftxui/component/component.hpp"         
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/component_options.hpp"
#include <ftxui/component/screen_interactive.hpp>
#include "ftxui/component/event.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/screen.hpp>

using namespace ftxui;

void show_message(const std::string& msg, Color c) {
    auto screen = ScreenInteractive::Fullscreen();
    std::vector<std::string> entries = {"OK"};
    int selected = 0;
    MenuOption option;
    option.on_enter = screen.ExitLoopClosure();
    auto menu = Menu(&entries, &selected, option);
    auto renderer = Renderer(menu, [&] {
        return vbox({
            text(msg) | color(c),
            separator(),
            menu->Render(),
        }) | border;
    });
    screen.Loop(renderer);
}