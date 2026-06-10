#include "types/tools.h"
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

void to_json(nlohmann::json& j, const ToolInfo& t) {
    if (t.thought_signature.has_value()) {
    j = nlohmann::json{{"id", t.id}, {"name", t.name},
    {"arguments", t.arguments}, {"thought_signature", t.thought_signature}};
    }
    else {
         j = nlohmann::json{{"id", t.id}, {"name", t.name},
    {"arguments", t.arguments}};
    }
}

void from_json(const nlohmann::json& j, ToolInfo& t) {
    j.at("id").get_to(t.id);
    j.at("name").get_to(t.name);
    j.at("arguments").get_to(t.arguments);
    if (j.contains("thought_signature")) {
        t.thought_signature = j["thoughtSignature"].get<std::string>();
    }
}

