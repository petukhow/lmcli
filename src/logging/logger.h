#pragma once

enum class LogLevel {
    Debug,
    Info,
    Error
};

void log(const LogLevel& level);