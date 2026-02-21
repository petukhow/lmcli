#include "utils.h"
#include "message.h"
#include <cstring>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>

bool limitExceeded(const std::vector<Message>& conversation, size_t limit) {
    if (limit == 0) return false;
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

std::string getSystemDataPath() {
    const char* dataDirs = std::getenv("XDG_DATA_DIRS");
    std::string firstDataDir;
    if (!dataDirs) {
        return "/usr/share/lmcli/"; // Fallback to system path
    }

    std::string dirsStr = dataDirs;
    while (true) {
        size_t colon = dirsStr.find(':');
        firstDataDir = (colon == std::string::npos) ? dirsStr : dirsStr.substr(0, colon);

        if (firstDataDir.empty()) return "/usr/share/lmcli/";

        if (std::filesystem::exists(firstDataDir + "/lmcli/")) {
            return firstDataDir + "/lmcli/";
        } else {
            dirsStr.erase(0, firstDataDir.length() + 1);
        }
    }   
    return firstDataDir + "/lmcli/";
}

void createConfigFileIfNotExists(const std::string& configDir, const std::string& fileTemplate) {
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

void createFileIfNotExists(const std::string& configDir, const std::string& fileTemplate) {
    size_t i = 1;
    std::string finalPath = configDir;
    while (std::filesystem::exists(finalPath)) {
        finalPath = configDir + "_" + std::to_string(i);
        i++;
    }   
    std::ofstream file(finalPath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not create " << finalPath << "\n";
        return; 
    }
    file << fileTemplate;
    std::cout << "✓ Created " << finalPath << "\n";
}


std::string getChatsDir() {
    const char* home = std::getenv("HOME");
    if (!home) {
        return "./";  // Fallback to current directory
    }
    return std::string(home) + "/.config/lmcli/chats/";
}

std::string getChatsPath(const std::string& filename) {
    const char* home = std::getenv("HOME");
    if (!home) {
        return "./" + filename;  // Fallback to current directory
    }
    return std::string(home) + "/.config/lmcli/chats" + filename;
}
