#include "tools.h"
#include "constants.h"
#include "loaders/config.h"
#include "logging/logger.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

static bool is_command_blacklisted(const nlohmann::json& cfg, const std::string& cmd) {
    for (const auto &command : cfg["blacklist"]) {
        bool is_blacklisted = cmd.find(command.get<std::string>()) != std::string::npos;
        if (is_blacklisted) {
            log(LogLevel::Info, "exec_bash declined: command is in blacklist: " + cmd);
            return true;
        }
    }
    return false;
}

static bool ask_confirmation(const std::string& cmd) {
    std::string answ;
    std::cerr << "\nModel wants to execute bash command: " << cmd << "\n";
    std::cerr << "Continue? (y/n) ";

    std::getline(std::cin, answ);
    
    if (!answ.empty() && std::tolower(answ[0]) == 'y') {
        return true;
    }
    return false;
}

static bool is_confirm_required(const nlohmann::json& cfg, const std::string& cmd) {
    bool is_confirm_required = false;
    if (cfg["confirm_required"] == "all") return true;

    if (!is_confirm_required) {
        for (const auto &command : cfg["confirm_required"]) {
            is_confirm_required = cmd.find(command.get<std::string>()) != std::string::npos;
        }
    }
    return is_confirm_required;
}

std::string exec_bash(const std::string& cmd) {
    auto cfg = load_config(CONFIG_FILE);
    
    if (is_command_blacklisted(cfg, cmd)) {
        return "command is not allowed";
    }

    if (is_confirm_required(cfg, cmd) && !ask_confirmation(cmd)) {
        return "command is not allowed by the user";
    }

    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        log(LogLevel::Error, "Pipe/fork failed in tools.cpp");
        return "error: pipe/fork failed";
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        log(LogLevel::Error, "Pipe/fork failed in tools.cpp");
        return "error: pipe/fork failed";
    }

    std::string result;
    if (pid == 0) {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        dup2(fd[1], STDERR_FILENO);
        close(fd[1]);
        log(LogLevel::Info, "Command will execute: " + cmd);
        execl("/bin/sh", "sh", "-c", cmd.c_str(), NULL);
        _exit(127);
    } else {
        close(fd[1]);

        char buf[4096];
        ssize_t bytes;
        while ((bytes = read(fd[0], buf, sizeof(buf))) > 0) {
            result += std::string(buf, bytes);
        }

        int status = 0;
        if (waitpid(pid, &status, 0) == -1) perror("waitpid");
    }

    return result;
}

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

