#include "../include/utils.h"
#include "../include/message.h"
#include <vector>

bool limitExceeded(const std::vector<Message>& conversation, size_t limit) {
    return conversation.size() >= limit;
}