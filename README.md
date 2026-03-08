# lmcli

A CLI tool for chatting with multiple LLM providers (Anthropic, OpenAI, etc.) from a single interface.

## Features

- 🤖 **Multi-provider support** — Anthropic, OpenAI, Groq, and any OpenAI-compatible API
- ⚙️ **Configurable** — System prompts, token limits, and conversation history
- 🔄 **Account switching** — Easily switch between different API accounts
- 💬 **Interactive chat** — Simple conversational interface with context management

## Supported Providers

- **Anthropic** (Claude models)
- **OpenAI-compatible** (GPT models, services: Groq, OpenRouter, etc.)
- **Google** (Gemini) - Untested due to regional restrictions

## Installation

### Prerequisites

- C++17 compiler (GCC 8+, Clang 7+, or MSVC 2019+)
- CMake 3.10+
- libcurl
- [nlohmann/json](https://github.com/nlohmann/json) (included in repo)

### Building from source
```bash
git clone https://github.com/petukhow/lmcli.git
cd lmcli
cmake -B build
cmake --build build
sudo cmake --install build
```

The binary will be located at `build/bin/lmcli`.

### Installing globally (optional)
```bash
sudo cp build/bin/lmcli /usr/local/bin/
```

## Quick Start

### 1. Initialize configuration
```bash
lmcli init
```

This creates the config directory at `~/.config/lmcli/` with template files.

### 2. Add an API account
```bash
lmcli setup
```

Follow the prompts to:
- Select a provider (Anthropic, OpenAI, Groq, etc.)
- Enter your API key
- Choose a model (or use the default)

### 3. Start chatting
```bash
lmcli start
```

Select an account and begin your conversation. Type `/exit` to quit.

## Usage
```
lmcli [COMMAND] [SUBCOMMAND]

Commands:
  init                              Initialize config directory and files
  setup                             Add a new provider account
  start                             Start a chat session
  accounts                          List configured accounts
  remove [account|chat|chats]       Remove account or chat(s)
  config [prompt|limit|max-tokens]  Show and edit current config settings
  help                              Show help message
  
Examples:
  lmcli init                # First-time setup
  lmcli setup               # Add a new account
  lmcli start               # Begin chatting (or just 'lmcli')
  lmcli accounts            # View your accounts
  lmcli remove account      # Remove an account
  lmcli remove chat         # Remove a chat
  lmcli remove chats        # Remove all chats
  lmcli config prompt       # Check and edit system prompt
  lmcli config limit        # Check and edit messages limit
  lmcli config max-tokens   # Check and edit max tokens limit
```

## Configuration

Configuration files are stored in `~/.config/lmcli/`:

### `config.json`
```json
{
  "system_prompt": "You are a helpful assistant.",
  "limit": 20,
  "max_tokens": 1024
}
```

- **system_prompt**: Default system message for all conversations
- **limit**: Maximum number of messages to keep in context (older messages are pruned). Set to 0 to disable
- **max_tokens**: Maximum tokens per API response

### `accounts.json`
Stores your configured API accounts (API keys, models, endpoints).

### `providers.json`
Defines available providers and their default settings. You can edit this to add custom OpenAI-compatible endpoints.

## Examples

### Using Anthropic's Claude
```bash
$ lmcli setup
Select a provider to set up (type '/exit' to quit):
-- Anthropic
-- OpenAI
-- Groq
> Anthropic

$ lmcli start
Select an account from the list below (type '/exit' to quit):
-- my-claude
> my-claude

Prompt (or '/exit' to end the conversation):
You: Hello!
Model: Hi! How can I help you today?
```

## Development

Built as a learning project to explore:
- Modern C++ (smart pointers, polymorphism, STL)
- External library integration (libcurl, nlohmann/json)
- CLI design patterns
- Multi-provider API abstraction
---
