#include "utils.h"
#include "message.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>

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

void createFileIfNotExists(const std::string& configDir, const std::string& fileTemplate) {
    if (std::filesystem::exists(configDir)) {
        std::cout << "⚠ " << configDir << " already exists, skipping\n";
        return;
    }
    
    // Create the file
    std::ofstream file(configDir);
    if (!file.is_open()) {
        std::cerr << "Error: Could not create " << configDir << "\n";
        return;
    }
    
    file << fileTemplate;
    std::cout << "✓ Created " << configDir << "\n";
}
size_t curlWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    std::string* response = static_cast<std::string*>(userdata);
    response->append(ptr, size * nmemb);

    return size * nmemb;
}