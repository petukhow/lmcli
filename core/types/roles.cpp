#include "roles.h"

std::string_view role_to_string(const Role &r) {
    switch (r) {
        case Role::Assistant: return "assistant";
        case Role::System: return "system";
        case Role::User: return "user";
        case Role::Tool: return "tool";
    };
}