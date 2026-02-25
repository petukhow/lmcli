#pragma once
#include <string>
#include <provider.h>

struct StreamContext {
    std::string buffer;
    const Provider* provider;               
};

void eventHandler(StreamContext* context);

