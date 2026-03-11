#include "streaming.h"
#include "provider.h"
#include <iostream>

size_t stream_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    std::cerr << "CHUNK: [" << std::string(ptr, size * nmemb) << "]\n";
    StreamContext* context = static_cast<StreamContext*>(userdata);
    context->buffer.append(ptr, size * nmemb);
    context->provider->event_handler(context);

    return size * nmemb;
}
