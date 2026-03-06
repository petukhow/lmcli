#include "message.h"
#include "json.hpp"

void to_json(nlohmann::json& j, const Message& m) {
    j = nlohmann::json{{"role", m.role}, {"content", m.content}, {"is_failed", m.is_failed}};
}

void from_json(const nlohmann::json& j, Message& m) {
    j.at("role").get_to(m.role);
    j.at("content").get_to(m.content);
    if (j.contains("is_failed")) {
        j.at("is_failed").get_to(m.is_failed);
    }
}