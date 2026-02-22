#include <string>
#include "json.hpp"
#pragma once

struct Message {
    std::string role;
    std::string content;
    bool isFailed = false;
};

void to_json(nlohmann::json& j, const Message& m);

void from_json(const nlohmann::json& j, Message& m);