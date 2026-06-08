#include <string>
#include "constants.h"
#include "loaders/config.h"
#include "command_policy.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "logging/logger.h"
#include "exec_bash.h"

std::string exec_bash(const std::string& cmd, const std::function<bool(const std::string&)>& confirm) {
    auto cfg = load_config(CONFIG_FILE);
    
    if (is_command_blacklisted(cfg, cmd)) {
        return "command is not allowed";
    }

    if (is_confirm_required(cfg, cmd) && !confirm(cmd)) {
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