#pragma once
#include <string>
extern bool logging;

enum class LogLevel {
    Debug,
    Info,
    Error
};

void log(LogLevel level, const std::string& event);

bool logger_init();