#include "tools.h"
#include "constants.h"
#include "loaders/config.h"
#include <cstdio>
#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

std::string read_file(const std::string filename) {
    std::ifstream file(filename); 

    if (!file.is_open()) {
        std::cerr << "Error: Could not open the file." << std::endl;
        return "";
    }

    std::ostringstream ss;
    ss << file.rdbuf();

    return ss.str();
}

std::string exec_bash(const std::string& cmd) {
    std::cerr << "[tool] " << cmd << "\n";

    auto cfg = load_config(CONFIG_FILE);
    std::string result;
    char buf[4096];
    ssize_t bytes;
    int fd[2];
    
    for (const auto &command : cfg["blacklist"]) {
        bool is_blacklisted = cmd.find(command.get<std::string>()) != std::string::npos;
        if (is_blacklisted) {
            return "command is not allowed.";
        }
    }

    for (const auto &command : cfg["confirm_required"]) {
        bool is_confirm_required = cmd.find(command.get<std::string>()) != std::string::npos;

        if (is_confirm_required) {
            std::string answ;
            std::cerr << "\nModel wants to execute bash command: " << cmd << "\n";
            std::cerr << "Continue? (y/n) ";

            if (!std::getline(std::cin, answ)) break;
            
            if (!answ.empty() && std::tolower(answ[0]) == 'y') {
                break;
            }
            else {
                return "command is not allowed by the user";
            }
        } 
    }

    if (pipe(fd) == -1) {
        perror("pipe");
        return "error: pipe/fork failed";
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return "error: pipe/fork failed";
    }

    if (pid == 0) {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        dup2(fd[1], STDERR_FILENO);
        close(fd[1]);
        execl("/bin/sh", "sh", "-c", cmd.c_str(), NULL);
        _exit(127);
    } else {
        close(fd[1]);

        while ((bytes = read(fd[0], buf, sizeof(buf))) > 0) {
            result += std::string(buf, bytes);
        }

        int status = 0;
        if (waitpid(pid, &status, 0) == -1) perror("waitpid");
    }

    return result;
}

void to_json(nlohmann::json& j, const ToolInfo& t) {
    j = nlohmann::json{{"id", t.id}, {"name", t.name}, {"arguments", t.arguments}};
}

void from_json(const nlohmann::json& j, ToolInfo& t) {
    j.at("id").get_to(t.id);
    j.at("name").get_to(t.name);
    j.at("arguments").get_to(t.arguments);
}

