#include "handlers/theme.h"
#include "constants.h"
#include "chat_flow.h"
#include "loaders/config.h"
#include "loaders/theme.h"
#include "logging/logger.h"
#include "json.hpp"
#include <string>
#include <utility>
#include <vector>

using json = nlohmann::json;

static void apply_theme(ChatSession& session, const std::string& theme_name) {
    session.theme = load_theme(theme_name);
    auto config = load_config(CONFIG_FILE);
    config["theme"] = theme_name;
    save_config(config);
}

static void populate_draft_from_json(ThemeDraft& draft, const json& entry) {
    draft.name = entry.at("name").get<std::string>();
    draft.user_color = entry.at("user_color").get<std::string>();
    draft.assistant_color = entry.at("assistant_color").get<std::string>();
    draft.status_color = entry.at("status_color").get<std::string>();
    draft.border_color = entry.at("border_color").get<std::string>();
    draft.prompt_color = entry.at("prompt_color").get<std::string>();
    draft.streaming_color = entry.at("streaming_color").get<std::string>();
    draft.separator_color = entry.at("separator_color").get<std::string>();
}

static void populate_draft_with_defaults(ThemeDraft& draft) {
    draft.name = "";
    draft.user_color = "cyan";
    draft.assistant_color = "white";
    draft.status_color = "yellow";
    draft.border_color = "white";
    draft.prompt_color = "cyan";
    draft.streaming_color = "white";
    draft.separator_color = "gray";
}

static void populate_draft_clone_from_current(ThemeDraft& draft, const json& themes_list, const std::string& current_name) {
    for (const auto& entry : themes_list) {
        if (entry.contains("name") && entry["name"] == current_name) {
            populate_draft_from_json(draft, entry);
            draft.name.clear();
            return;
        }
    }
    populate_draft_with_defaults(draft);
}

static bool submit_theme_form(ChatSession& session, const std::string& original_name) {
    auto& draft = session.theme_draft;
    auto themes_data = load_themes_file();
    auto& themes_list = themes_data["themes"];

    bool has_errors = false;

    if (draft.name.empty()) {
        draft.name_error = "Theme name cannot be empty";
        has_errors = true;
    } else if (draft.name != original_name && theme_name_exists(themes_data, draft.name)) {
        draft.name_error = "A theme with this name already exists";
        has_errors = true;
    } else {
        draft.name_error = std::nullopt;
    }

    const std::vector<std::pair<std::string, std::string>> fields = {
        {"User", draft.user_color},
        {"Assistant", draft.assistant_color},
        {"Status", draft.status_color},
        {"Border", draft.border_color},
        {"Prompt", draft.prompt_color},
        {"Streaming", draft.streaming_color},
        {"Separator", draft.separator_color},
    };
    std::string bad_fields;
    for (const auto& [label, value] : fields) {
        if (!is_valid_color_name(value)) {
            if (!bad_fields.empty()) bad_fields += ", ";
            bad_fields += label;
        }
    }
    if (!bad_fields.empty()) {
        draft.color_error = bad_fields + " must be one of: " + valid_color_names();
        has_errors = true;
    } else {
        draft.color_error = std::nullopt;
    }

    if (has_errors) return false;

    json entry;
    entry["name"] = draft.name;
    entry["user_color"] = draft.user_color;
    entry["assistant_color"] = draft.assistant_color;
    entry["status_color"] = draft.status_color;
    entry["border_color"] = draft.border_color;
    entry["prompt_color"] = draft.prompt_color;
    entry["streaming_color"] = draft.streaming_color;
    entry["separator_color"] = draft.separator_color;

    bool replaced = false;
    if (!original_name.empty()) {
        for (auto& t : themes_list) {
            if (t.contains("name") && t["name"] == original_name) {
                t = entry;
                replaced = true;
                break;
            }
        }
    }
    if (!replaced) themes_list.push_back(entry);

    save_themes_file(themes_data);
    apply_theme(session, draft.name);
    log(LogLevel::Info, "Theme saved: " + draft.name);
    return true;
}

void open_theme_menu(ChatSession& session) {
    auto themes_data = load_themes_file();
    auto themes_list = themes_data["themes"];

    session.menu_settings.menu_cursor = 0;
    session.menu_settings.menu_items.clear();
    session.menu_settings.title = "Current theme: " + session.theme.name + " — select to apply/edit, or create a new one:";
    session.prompt.content.clear();

    session.menu_settings.menu_items.push_back("+ Create new theme");
    for (const auto& t : themes_list) {
        session.menu_settings.menu_items.push_back(t["name"].get<std::string>());
    }

    session.menu_settings.on_select = [&session, themes_list](size_t cursor) {
        session.theme_draft = {};
        std::string original_name;

        if (cursor == 0) {
            populate_draft_clone_from_current(session.theme_draft, themes_list, session.theme.name);
        } else {
            const auto& entry = themes_list[cursor - 1];
            populate_draft_from_json(session.theme_draft, entry);
            original_name = session.theme_draft.name;
        }

        session.form.on_submit = [&session, original_name]() -> bool {
            return submit_theme_form(session, original_name);
        };

        session.mode = Mode::Form;
        session.active_tab = 4;
    };

    session.menu_settings.on_delete = [&session](size_t cursor) {
        if (cursor == 0) return; // "+ Create new theme" isn't deletable

        auto themes_data = load_themes_file();
        auto& themes_list = themes_data["themes"];
        const size_t idx = cursor - 1;
        if (idx >= themes_list.size()) return;

        const std::string deleted_name = themes_list[idx]["name"].get<std::string>();
        themes_list.erase(themes_list.begin() + static_cast<long>(idx));
        save_themes_file(themes_data);

        if (session.theme.name == deleted_name) {
            const std::string fallback = !themes_list.empty()
                ? themes_list[0]["name"].get<std::string>() : "tech";
            apply_theme(session, fallback);
        }

        session.menu_settings.menu_items.erase(session.menu_settings.menu_items.begin() + static_cast<long>(cursor));
        auto& c = session.menu_settings.menu_cursor;
        if (c >= session.menu_settings.menu_items.size() && c > 0) c--;

        log(LogLevel::Info, "Theme deleted: " + deleted_name);
    };

    session.mode = Mode::Menu;
}

using namespace ftxui;

Element build_theme_errors_block(const ChatSession& session) {
    Elements errors;
    if (session.active_form == "/theme") {
        if (session.theme_draft.name_error.has_value())
            errors.push_back(hbox({text(""), text(*session.theme_draft.name_error) | color(Color::Red)}));
        if (session.theme_draft.color_error.has_value())
            errors.push_back(hbox({text(""), paragraph(*session.theme_draft.color_error) | color(Color::Red)}));
    }
    return vbox(std::move(errors));
}

static Element color_row(const ChatSession& session, const std::string& label,
    const ftxui::Component& input, const std::string& value) {
    return hbox({
        text(label) | size(WIDTH, EQUAL, 20) | color(session.theme.prompt_color),
        text("› ") | color(session.theme.prompt_color),
        input->Render(),
        text(" "),
        text("  ") | bgcolor(color_from_string(value)),
    });
}

Element render_theme_form(const ChatSession& session, const ThemeFields& theme_fields, const Element& theme_errors_block) {
    const auto& draft = session.theme_draft;
    return vbox({
        hbox({text("Theme name") | size(WIDTH, EQUAL, 20) | color(session.theme.prompt_color),
            text("› ") | color(session.theme.prompt_color), theme_fields.name_input->Render()}),
        color_row(session, "User color", theme_fields.user_color_input, draft.user_color),
        color_row(session, "Assistant color", theme_fields.assistant_color_input, draft.assistant_color),
        color_row(session, "Status color", theme_fields.status_color_input, draft.status_color),
        color_row(session, "Border color", theme_fields.border_color_input, draft.border_color),
        color_row(session, "Prompt color", theme_fields.prompt_color_input, draft.prompt_color),
        color_row(session, "Streaming color", theme_fields.streaming_color_input, draft.streaming_color),
        color_row(session, "Separator color", theme_fields.separator_color_input, draft.separator_color),
        separator() | color(session.theme.separator_color),
        theme_errors_block,
        text(""),
        paragraph("Available colors: " + valid_color_names()) | color(Color::GrayDark),
        paragraph("Сtrl + S to save & apply | esc to exit") | color(Color::GrayDark),
        text("")
    });
}
