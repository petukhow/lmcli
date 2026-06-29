#include "constants.h"
#include "chat_flow.h"
#include "loaders/config.h"
#include "json.hpp"

using json = nlohmann::json;

void open_config_menu(ChatSession& session) {
    auto config = load_config(CONFIG_FILE);
    session.config_draft.system_prompt = config["system_prompt"].get<std::string>();
    session.config_draft.limit = std::to_string(config["limit"].get<size_t>());
    session.config_draft.max_tokens = std::to_string(config["max_tokens"].get<size_t>());
    session.config_draft.logging = config["logging"].get<bool>();
    if (config["confirm_required"].is_string()) {
        session.config_draft.confirm_required_raw = config["confirm_required"].get<std::string>();
    } else {
        session.config_draft.confirm_required_raw = config["confirm_required"].dump();
    }
    session.config_draft.restricted_raw = config["blacklist"].dump();

    session.form.on_submit = [&session]() -> bool {
        auto& draft = session.config_draft;
        bool has_errors = false;
        try {
            int limit = std::stoi(draft.limit);
            if (limit < 0) {
                draft.limit_error = "Limit must be a positive number";
                has_errors = true;
            }
            else draft.limit_error = std::nullopt;
        } catch (const std::invalid_argument&) {
            draft.limit_error = "Limit must be a number";
            has_errors = true;
        }

        try {
            int max_tokens = std::stoi(draft.max_tokens);
            if (max_tokens < 0) {
                draft.max_tokens_error = "Max tokens must be a positive number";
                has_errors = true;
            }
            else draft.max_tokens_error = std::nullopt;
        } catch (const std::invalid_argument&) {
            draft.max_tokens_error = "Max tokens must be a number";
            has_errors = true;
        }

        json confirm_json, blacklist_json;
        if (draft.confirm_required_raw == "all") confirm_json = "all";
        else {
            try {
                confirm_json = json::parse(draft.confirm_required_raw);
                if (!confirm_json.is_array()) {
                    draft.confirm_required_error = "Confirm required commands must be a JSON array";
                    has_errors = true;
                }
            } catch (const json::parse_error&) {
                draft.confirm_required_error = "Confirm required commands field has invalid JSON";
                has_errors = true;
            }
        }

        try {
            blacklist_json = json::parse(draft.restricted_raw);
            if (!blacklist_json.is_array()) {
                draft.restricted_error = "Restricted commands must be a JSON array";
                has_errors = true;
            }
        } catch (const json::parse_error&) {
            draft.restricted_error = "Restricted commands field has invalid JSON";
            has_errors = true;
        }

        if (has_errors) return false;

        json config = load_config(CONFIG_FILE);
        config["system_prompt"] = draft.system_prompt;
        config["limit"] = std::stoi(draft.limit);
        config["max_tokens"] = std::stoi(draft.max_tokens);
        config["logging"] = draft.logging;
        config["confirm_required"] = confirm_json;
        config["blacklist"] = blacklist_json;
        save_config(config);

        return true;
    };

    session.mode = Mode::Form;
    session.active_tab = 2;
}