#pragma once
#include <string>

class Provider;

struct StreamContext {
    std::string buffer;
    std::string full_content;
    const Provider* provider;
    bool is_failed = false;
};

size_t stream_callback(char *ptr, size_t size, size_t nmemb, void *userdata);

