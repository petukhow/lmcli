#pragma once
#include <string>

class Provider;

struct StreamContext {
    std::string buffer;
    std::string fullContent;
    const Provider* provider;  
    bool isFailed = false;
};

size_t streamCallback(char *ptr, size_t size, size_t nmemb, void *userdata);

