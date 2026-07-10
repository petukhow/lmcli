# lmcli

A terminal UI for chatting with multiple LLM providers (Anthropic, OpenAI, etc.) from a single interface.

## Features

- **Multi-provider support** — Anthropic, OpenAI, Groq, Google, OpenRouter, and any OpenAI-compatible API
- **Configurable** — System prompts, token limits, and conversation history
- **Account switching** — Easily switch between different API accounts
- **Interactive TUI** — FTXUI-based chat interface with context management
- **Agent mode** — Models can call tools (e.g. shell commands) across multiple turns, with confirmation/blacklist safety checks (see [Safety notes](#safety-notes) below)
- **Streaming responses** — Get the model's answer instantly token-by-token
- **Custom themes** — Create, edit, and switch color themes with named colors or hex codes

## Supported Providers

- **Anthropic** (Claude models)
- **Any OpenAI-compatible** (GPT models, services)
- **OpenRouter**
- **Groq**
- **Google** (Gemini)

## Installation

### Prerequisites

- C++17 compiler (GCC 8+, Clang 7+)
- CMake 3.15+
- libcurl
- [nlohmann/json](https://github.com/nlohmann/json) (included in repo)

### Install script

```bash
curl -fsSL https://raw.githubusercontent.com/petukhow/lmcli/master/install.sh | bash
```

This clones the repo, builds it, and installs it to `/usr/local` (asks for `sudo` at the install step). It checks for `git`, `cmake`, a C++17 compiler, and `libcurl` up front and tells you what's missing rather than installing anything for you. If you'd rather not pipe a script into `bash`, it's a plain, short file — read it first: [`install.sh`](install.sh).

### Building from source manually
```bash
git clone https://github.com/petukhow/lmcli.git
cd lmcli
cmake -B build
cmake --build build
sudo cmake --install build
```

The binary will be located at `build/bin/lmcli`.

## Quick Start

### 1. Start lmcli
```bash
lmcli
# or explicitly: lmcli start
```
On first run, this automatically creates the config directory at `~/.config/lmcli/` with template files — no separate init step needed. (`lmcli init` also exists if you want to (re)create the config files without starting a chat.)

### 2. Add an API account
Inside the app, run:
```
/setup
```
Follow the prompts to select a provider, enter your API key, and choose a model (or use the default).

### 3. Chat
Once an account is selected, just type a message and press enter. Type `/exit` to quit.

## Usage

```
lmcli [COMMAND]

Commands:
  start   Start a chat session (default when no command is given)
  init    Initialize config directory and files
  help    Show help message
```

Everything else — accounts, chats, settings, themes — is handled from in-chat commands, typed directly into the prompt:

| Command    | Description                                              |
|------------|-----------------------------------------------------------|
| `/setup`   | Add a new provider account                                |
| `/account` | Switch between configured accounts (press `d` `d` to delete one) |
| `/chats`   | Browse, open, or create saved chats (press `d` `d` to delete one) |
| `/config`  | Edit settings (system prompt, limits, confirmations, ...)  |
| `/theme`   | Create, edit, clone, or switch color themes (press `d` `d` to delete one) |
| `/exit`    | Quit lmcli, or close the current chat                      |

`ctrl+o` toggles whether tool call output (e.g. `exec_bash` results) is shown inline or collapsed to a one-line summary — handy when a tool call dumps a large file and you just want to keep reading the conversation.

## Configuration

Configuration files are stored in `~/.config/lmcli/`:

### `config.json`
```json
{
  "system_prompt": "You're a helpful assistant.",
  "limit": 20,
  "max_tokens": 1024,
  "logging": true,
  "blacklist": ["reboot", "shutdown", "poweroff", "halt", "init 0", "init 6"],
  "confirm_required": "all",
  "theme": "tech",
  "current_account": ""
}
```

- **system_prompt**: Default system message for all conversations
- **limit**: Maximum number of messages to keep in context (older messages are pruned). Set to 0 to disable
- **max_tokens**: Maximum tokens per API response
- **logging**: Store logs locally (logs are written to `~/.local/state/lmcli/`)
- **blacklist**: Commands the model is never allowed to execute
- **confirm_required**: `"all"` to require confirmation for every tool call, or a JSON array of specific commands that require confirmation
- **theme**: Name of the active theme (see `themes.json` below)

All of the above can be edited live from the `/config` command.

### `accounts.json`
Stores your configured API accounts (API keys, models, endpoints), written with `0600` permissions. Managed via `/setup` and `/account`.

### `providers.json`
Defines available providers and their default settings. You can edit this to add custom OpenAI-compatible endpoints.

### `themes.json`
Stores your custom color themes, managed via `/theme`. Each theme defines colors for the user/assistant text, prompt, status bar, borders, streaming text, and separators:
```json
{
  "themes": [
    {
      "name": "tech",
      "user_color": "cyan",
      "assistant_color": "white",
      "status_color": "white",
      "border_color": "white",
      "prompt_color": "cyan",
      "streaming_color": "white",
      "separator_color": "gray"
    }
  ]
}
```
Colors can be one of the built-in names (`black`, `red`, `green`, `yellow`, `blue`, `magenta`, `cyan`, `white`, `gray`, and their `_light` variants) or a hex code like `#ff8800`.

### `tools.json`
Stores available tools for models (e.g. `exec_bash`).

## Safety notes

Agent mode lets the model run shell commands on your machine via `exec_bash`. The `blacklist` and `confirm_required` settings are a basic guardrail, not a sandbox — they match on substrings, so a determined or confused model can still phrase its way around them. Don't run agent mode unattended on anything you're not prepared to have modified, and review commands before confirming them.

## Example

```
$ lmcli
lmcli
v1.0.0

Type a message to start chatting, or use:
  /setup   configure an account
  /chats   browse saved chats
  /config  edit settings
  /theme   create or switch color themes
  /exit    quit

› /setup
```
After picking a provider and entering an API key, just start typing to chat:
```
› Hello!

Hi! How can I help you today?
```
