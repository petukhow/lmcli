#pragma once
#include <string>

namespace ansi {
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
    const std::string CYAN = "\033[36m";
    const std::string BLINK = "\033[5;91m";
    const std::string BRED = "\033[91m";
    const std::string END = "\033[0m";

    const std::string UP = "\033[A";
    const std::string CLEAR = "\033[2K";
}