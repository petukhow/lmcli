#pragma once
#include <string>

struct ToolInfo {
    std::string id;
    std::string name;
    std::string arguments;
};

std::string read_file(const std::string filename);