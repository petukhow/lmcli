#pragma once
#include <functional>
#include <string>
#include <vector>

enum class Mode {
    Main,
    Menu,
    Form
};

struct Form {
    std::string acc_name;
    std::string api_key;
    std::string model_name;
};

struct MenuSettings {
    std::vector<std::string> menu_items;
    std::function<void(int)> on_select;
    int menu_cursor;
};
