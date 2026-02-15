#include "utils.h"
#include "message.h"
#include <vector>

bool limitExceeded(const std::vector<Message>& conversation, size_t limit) {
    return conversation.size() >= limit;
}

std::string getConfigPath(const std::string& filename) {
    const char* home = std::getenv("HOME");
    if (!home) {
        return "./" + filename;  // Fallback to current directory
    }
    return std::string(home) + "/.config/lmcli/" + filename;
}

size_t curlWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    std::string* response = static_cast<std::string*>(userdata);
    response->append(ptr, size * nmemb);

    return size * nmemb;
}