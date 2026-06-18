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
    std::string default_name;
    std::string default_model;
    std::function<void()> on_submit;
};

struct MenuSettings {
    std::vector<std::string> menu_items;
    std::function<void(size_t)> on_select;
    size_t menu_cursor = 0;
};
