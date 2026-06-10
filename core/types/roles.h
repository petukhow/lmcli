#pragma once
#include <string_view>

enum class Role {
    Assistant,
    User,
    Tool,
    System
};

std::string_view role_to_string(const Role& r);