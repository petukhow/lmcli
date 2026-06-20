#pragma once
#include "types/accounts.h"
#include <functional>
#include <optional>
#include <string>
#include <vector>

enum class Mode {
    Main,
    Menu,
    Form
};

struct Form {
    std::optional<std::string> key_error = std::nullopt;
    std::optional<std::string> name_error = std::nullopt;
    std::string default_name;
    std::string default_model;
    std::function<void()> on_submit;
    AccountDraft draft;
};

struct MenuSettings {
    std::vector<std::string> menu_items;
    std::function<void(size_t)> on_select;
    size_t menu_cursor = 0;
};
