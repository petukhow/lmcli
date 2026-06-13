#pragma once
#include "json.hpp"
#include <string>

const std::string CONFIG_FILE = "config.json";
const std::string ACCOUNTS_FILE = "accounts.json";
const std::string PROVIDERS_FILE = "providers.json";
const std::string TOOLS_FILE = "tools.json";
const std::string THEMES_FILE = "themes.json";
const std::string LOGS_FILE = "lmcli.log";

const nlohmann::json ACCOUNTS_DEFAULT = {{"accounts", nlohmann::json::array({})}};
const nlohmann::json CONFIG_DEFAULT = {
        {"system_prompt", "You're a helpful assistant."},
        {"limit", 20},
        {"max_tokens", 1024},
        {"logging", true},
        {"blacklist", nlohmann::json::array({"reboot", "shutdown", "poweroff", "halt", "init 0", "init 6"})},
        {"confirm_required", "all"},
        {"theme", "tech"}
};
const nlohmann::json TOOLS_DEFAULT = {
    {"tools", nlohmann::json::array({
        {
            {"name", "exec_bash"},
            {"description", "Execute a bash command and return its output. Use this for running commands, writing files, compiling code, etc."},
            {"input_schema", {
                {"type", "object"},
                {"properties", {
                    {"command", {
                        {"type", "string"},
                        {"description", "Bash command to execute"}
                    }}
                }},
                {"required", nlohmann::json::array({"command"})}
            }}
        }
    })}
};
const nlohmann::json PROVIDERS_DEFAULT = {
    {"providers", nlohmann::json::array({
        {
            {"name", "Anthropic"},
            {"type", "anthropic"},
            {"default_api_url", "https://api.anthropic.com/v1/messages"},
            {"default_model", "claude-sonnet-4-20250514"}
        },
        {
            {"name", "Groq"},
            {"type", "openai-compatible"},
            {"default_api_url", "https://api.groq.com/openai/v1/chat/completions"},
            {"default_model", "llama-3.3-70b-versatile"}
        },
        {
            {"name", "OpenAI"},
            {"type", "openai-compatible"},
            {"default_api_url", "https://api.openai.com/v1/chat/completions"},
            {"default_model", "gpt-5.2"}
        },
        {
            {"name", "Google"},
            {"type", "google"},
            {"default_api_url", "https://generativelanguage.googleapis.com/v1beta/models/"},
            {"default_model", "gemini-3-flash-preview"}
        },
        {
            {"name", "OpenRouter"},
            {"type", "openai-compatible"},
            {"default_api_url", "https://openrouter.ai/api/v1/chat/completions"},
            {"default_model", "moonshotai/kimi-k2.6:free"}
        }
    })}
};
const nlohmann::json CHAT_DEFAULT = {{"conversation", nlohmann::json::array({})}};
