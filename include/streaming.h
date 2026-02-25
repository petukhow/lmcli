#pragma once
#include <string>

class Provider;

struct StreamContext {
    std::string buffer;
    const Provider* provider;               
};

void eventHandler(StreamContext* context);

size_t streamCallback(char *ptr, size_t size, size_t nmemb, void *userdata);

