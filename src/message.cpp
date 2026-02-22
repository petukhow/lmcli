#include "message.h"
#include "json.hpp"

void to_json(nlohmann::json& j, const Message& m) {
    j = nlohmann::json{{"role", m.role}, {"content", m.content}, {"isFailed", m.isFailed}};
}

void from_json(const nlohmann::json& j, Message& m) {
    j.at("role").get_to(m.role);
    j.at("content").get_to(m.content);
    if (j.contains("isFailed")) {
        j.at("isFailed").get_to(m.isFailed);
    }
}