#include "streaming.h"
#include "provider.h"

size_t streamCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    StreamContext* context = static_cast<StreamContext*>(userdata);
    context->buffer.append(ptr, size * nmemb);
    context->provider->eventHandler(context);

    return size * nmemb;
}