#pragma once
#include <string>
#include "tools.h"
#include <vector>

class Provider;

struct StreamContext {
    std::string buffer;
    std::string tool_buffer;
    std::string full_content;
    ToolInfo pending_tool;
    const Provider* provider;
    std::vector<ToolInfo> tool_calls; 
    bool is_failed = false;
};

size_t stream_callback(char *ptr, size_t size, size_t nmemb, void *userdata);

