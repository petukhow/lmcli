#include "provider.h"
#include "logging/logger.h"

size_t stream_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    StreamContext* context = static_cast<StreamContext*>(userdata);
    if (context->cancelled && context->cancelled->load()) {
        return 0;
    }
    context->buffer.append(ptr, size * nmemb);
    log(LogLevel::Debug, "raw buffer: " + context->buffer);
    context->provider->event_handler(context, context->callback);

    return size * nmemb;
}
