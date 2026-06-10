#include "roles.h"

std::string_view role_to_string(const Role &r) {
    std::string_view string_role;
    switch (r) {
        case Role::Assistant: string_role = "assistant";
        case Role::System: string_role = "system";
        case Role::User: string_role = "user";
        case Role::Tool: string_role = "tool";
    }
    return string_role;
}