#pragma once
#include <string>
#include <vector>
#include "json.hpp"
#include "tools.h"

struct Message {
    std::string role;
    std::string content;
    std::string tool_call_id;
    std::vector<ToolInfo> tool_calls;
    bool is_failed = false;
};

void to_json(nlohmann::json& j, const Message& m);

void from_json(const nlohmann::json& j, Message& m);