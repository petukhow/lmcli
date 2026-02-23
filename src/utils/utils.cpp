#include "utils.h"
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>

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

std::string getSystemDataPath(const std::string& filename) {
    const char* dataDirs = std::getenv("XDG_DATA_DIRS");
    std::string firstDataDir;
    if (!dataDirs) {
        return "/usr/share/lmcli/" + filename; // Fallback to system path
    }

    std::string dirsStr = dataDirs;
    while (true) {
        size_t colon = dirsStr.find(':');
        firstDataDir = (colon == std::string::npos) ? dirsStr : dirsStr.substr(0, colon);

        if (firstDataDir.empty()) return "/usr/share/lmcli/";

        std::string base = firstDataDir.back() == '/' ? firstDataDir : firstDataDir + "/";
        std::string cfg  = base + "lmcli/" + filename;
        if (std::filesystem::exists(cfg)) {
            return base + "lmcli/" + filename;
        } else {
            dirsStr.erase(0, firstDataDir.length() + (colon == std::string::npos ? 0 : 1));
            if (colon == std::string::npos) break;
        }
    }   
    return "/usr/share/lmcli/" + filename;
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

std::string createFileIfNotExists(const std::string& chatsDir, const std::string& fileTemplate) {
    size_t i = 1;
    std::filesystem::path finalPath = chatsDir;
    std::string stem = finalPath.parent_path() / finalPath.stem();
    std::string extension = finalPath.extension().string();

    while (std::filesystem::exists(finalPath)) {
        finalPath = stem + "_" + std::to_string(i) + extension;
        i++;
    }   
    std::ofstream file(finalPath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not create " << finalPath << "\n";
        return ""; 
    }
    file << fileTemplate;
    std::cout << "✓ Created " << finalPath << "\n";
    return finalPath.string();
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
    return std::string(home) + "/.config/lmcli/chats/" + filename;
}
