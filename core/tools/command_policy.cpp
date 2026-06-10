#include "command_policy.h"
#include "logging/logger.h"
#include "json.hpp"

bool is_confirm_required(const nlohmann::json& cfg, const std::string& cmd) {
    bool is_confirm_required = false;
    if (cfg["confirm_required"] == "all") return true;

    for (const auto &command : cfg["confirm_required"]) {
        is_confirm_required |= cmd.find(command.get<std::string>()) != std::string::npos;
    }
    return is_confirm_required;
}

bool is_command_blacklisted(const nlohmann::json& cfg, const std::string& cmd) {
    for (const auto &command : cfg["blacklist"]) {
        bool is_blacklisted = cmd.find(command.get<std::string>()) != std::string::npos;
        if (is_blacklisted) {
            log(LogLevel::Info, "exec_bash declined: command is in blacklist: " + cmd);
            return true;
        }
    }
    return false;
}