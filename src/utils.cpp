#include "utils.h"
#include "message.h"
#include <vector>
#include <fstream>
#include <iostream>

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

std::string getConfigDir() {
    const char* home = std::getenv("HOME");
    if (!home) {
        return "./"; // Fallback to current directory
    }
    return std::string(home) + "/.config/lmcli/";
}

void createFileIfNotExists(const std::string& path, const std::string& content) {
    if (std::filesystem::exists(path)) {
        std::cout << "⚠ " << path << " already exists, skipping\n";
        return;
    }
    
    // Create the file
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not create " << path << "\n";
        return;
    }
    
    file << content;
    std::cout << "✓ Created " << path << "\n";
}
size_t curlWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    std::string* response = static_cast<std::string*>(userdata);
    response->append(ptr, size * nmemb);

    return size * nmemb;
}