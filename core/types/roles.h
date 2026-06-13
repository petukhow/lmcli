#pragma once
#include <string_view>
#include <string>
#include "json.hpp"

enum class Role {
    Assistant,
    User,
    Tool,
    System
};

std::string_view role_to_string(const Role& r);
Role string_to_role(const std::string& s);

inline void to_json(nlohmann::json& j, const Role& r) {
    j = std::string(role_to_string(r));
}

inline void from_json(const nlohmann::json& j, Role& r) {
    r = string_to_role(j.get<std::string>());
}