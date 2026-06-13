#include "roles.h"

std::string_view role_to_string(const Role &r) {
    switch (r) {
        case Role::Assistant: return "assistant";
        case Role::System: return "system";
        case Role::User: return "user";
        case Role::Tool: return "tool";
    }
    return "unknown";
}

Role string_to_role(const std::string& s) {
    if (s == "assistant") return Role::Assistant;
    if (s == "system") return Role::System;
    if (s == "user") return Role::User;
    if (s == "tool") return Role::Tool;
    return Role::User;
}