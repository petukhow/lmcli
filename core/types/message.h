#pragma once
#include <string>
#include <vector>
#include "roles.h"
#include "json.hpp"
#include "tools.h"

struct Message {
    Role role;
    std::string content;
    std::string tool_call_id;
    std::vector<ToolInfo> tool_calls;
    bool is_failed = false;
};

inline void trim_history(std::vector<Message>& conv, size_t limit) {
    while (limit < conv.size()) {
        if (conv.size() < 2) return;

        conv.erase(conv.begin()+1);
        // we need to remove all the tool calls with this id so it doesn't break during API request
        while (conv.size() >= 2 && conv[1].role == Role::Tool) {
            conv.erase(conv.begin()+1);
        }
        continue;
    }
    return;
}

inline void to_json(nlohmann::json& j, const Message& m) {
    j = nlohmann::json{{"role", role_to_string(m.role)}, {"content", m.content},
    {"is_failed", m.is_failed}, {"tool_call_id", m.tool_call_id}, {"tool_calls", m.tool_calls}};
}

inline void from_json(const nlohmann::json& j, Message& m) {
    j.at("role").get_to(m.role);
    j.at("content").get_to(m.content);

    if (j.contains("is_failed")) {
        j.at("is_failed").get_to(m.is_failed);
    }
    if (j.contains("tool_call_id")) {
        j.at("tool_call_id").get_to(m.tool_call_id);
    }
    if (j.contains("tool_calls")) {
        j.at("tool_calls").get_to(m.tool_calls);
    }
}