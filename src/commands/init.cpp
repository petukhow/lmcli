#include "utils.h"
#include "commands.h"
#include <iostream>
#include <string>
#include <filesystem>


void init() {
    std::string configDir = getConfigDir();

    try {
        std::filesystem::create_directory(configDir);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return;
    }

    const char* configTemplate = R"({
        "system_prompt": "You are a helpful assistant.",
        "limit": 20,
        "max_tokens": 1024
    })";
    const char* accountsTemplate = R"({
        "accounts": []
    })";
    const char* providersTemplate = R"({
        "providers": [
            {
            "name": "Anthropic",
            "type": "anthropic",
            "default_api_url": "https://api.anthropic.com/v1/messages",
            "default_model": "claude-sonnet-4-20250514"
            },
            {
            "name": "OpenAI",
            "type": "openai-compatible",
            "default_api_url": "https://api.openai.com/v1/chat/completions",
            "default_model": "gpt-4"
            },
            {
           "name": "Groq",
            "type": "openai-compatible",
            "default_api_url": "https://api.groq.com/openai/v1/chat/completions",
            "default_model": "llama-3.3-70b-versatile"
            }
            ]
        })";

    createFileIfNotExists(configDir + "providers.json", providersTemplate);
    createFileIfNotExists(configDir + "config.json", configTemplate);
    createFileIfNotExists(configDir + "accounts.json", accountsTemplate);

    std::cout << "\n✓ Initialization complete!\n";
    std::cout << "Next steps:\n";
    std::cout << "  1. Run 'lmcli setup' to add an account\n";
    std::cout << "  2. Run 'lmcli start' to begin chatting\n";
}