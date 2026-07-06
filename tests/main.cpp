#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <filesystem>
#include <fstream>
#include "providers/anthropic.h"
#include "providers/google.h"
#include "providers/open_ai_compatible.h"
#include "tools/command_policy.h"
#include "types/theme.h"
#include "loaders/json_io.h"
#include "json.hpp"

TEST_CASE("Anthropic extract_delta with valid response") {
    auto anthropic = Anthropic("", "", "", "", 0, 0);
    nlohmann::json response = {
    {"delta", {{"text", "hello"}}}
    };

    auto result = anthropic.extract_delta(response);
    CHECK(result.has_value());
    CHECK(result == "hello");
}

TEST_CASE("Google extract_delta with valid response") {
    auto google = Google("", "", "", "", 0, 0);
    nlohmann::json response = {
    {"candidates", {{
        {"content", {{"parts", {{{"text", "hello"}}}}}}
        }}}
    };

    auto result = google.extract_delta(response);
    CHECK(result.has_value());
    CHECK(result == "hello");
}

TEST_CASE("OpenAI compatible extract_delta with valid response") {
    auto openai_comp = OpenAICompatible("", "", "", "", 0, 0);
    nlohmann::json response = {
    {"choices", {{
        {"delta", {{"content", "hello"}}}
    }}}
};

    auto result = openai_comp.extract_delta(response);
    CHECK(result.has_value());
    CHECK(result == "hello");
}

TEST_CASE("Anthropic extract_delta with empty response") {
    auto anthropic = Anthropic("", "", "", "", 0, 0);
    nlohmann::json response = {};

    auto result = anthropic.extract_delta(response);
    CHECK(result == std::nullopt);
}

TEST_CASE("Google extract_delta with empty response") {
    auto google = Google("", "", "", "", 0, 0);
    nlohmann::json response = {};

    auto result = google.extract_delta(response);
    CHECK(result == std::nullopt);
}

TEST_CASE("OpenAI compatible extract_delta with empty response") {
    auto openai_comp = OpenAICompatible("", "", "", "", 0, 0);
    nlohmann::json response = {};

    auto result = openai_comp.extract_delta(response);
    CHECK(result == std::nullopt);
}

TEST_CASE("OpenAI-compatible's extract_delta accesses to an empty array") {
    auto openai_comp = OpenAICompatible("", "", "", "", 0, 0);
    nlohmann::json response = {{"choices", {}}};

    auto result = openai_comp.extract_delta(response);
    CHECK(result == std::nullopt);
}

TEST_CASE("Google's extract_delta accesses to an empty array") {
    auto google = Google("", "", "", "", 0, 0);
    nlohmann::json response = {{"candidates", {}}};

    auto result = google.extract_delta(response);
    CHECK(result == std::nullopt);
}

TEST_CASE("Anthropic's extract_delta accesses to an empty array") {
    auto anthropic = Anthropic("", "", "", "", 0, 0);
    nlohmann::json response = {{"delta", {}}};

    auto result = anthropic.extract_delta(response);
    CHECK(result == std::nullopt);
}

TEST_CASE("is_confirm_required returns true when confirm_required is 'all'") {
    nlohmann::json cfg = {{"confirm_required", "all"}};
    CHECK(is_confirm_required(cfg, "ls -la"));
}

TEST_CASE("is_confirm_required returns true for a matching command in the list") {
    nlohmann::json cfg = {{"confirm_required", nlohmann::json::array({"rm", "mv"})}};
    CHECK(is_confirm_required(cfg, "rm -rf /tmp/foo"));
}

TEST_CASE("is_confirm_required returns false when no command matches") {
    nlohmann::json cfg = {{"confirm_required", nlohmann::json::array({"rm", "mv"})}};
    CHECK_FALSE(is_confirm_required(cfg, "ls -la"));
}

TEST_CASE("is_confirm_required returns false for an empty list") {
    nlohmann::json cfg = {{"confirm_required", nlohmann::json::array({})}};
    CHECK_FALSE(is_confirm_required(cfg, "rm -rf /"));
}

TEST_CASE("is_confirm_required matches substrings, not whole words") {
    // documents current behavior: "rm" also matches inside unrelated words
    nlohmann::json cfg = {{"confirm_required", nlohmann::json::array({"rm"})}};
    CHECK(is_confirm_required(cfg, "warmup_script.sh"));
}

TEST_CASE("is_command_blacklisted returns true for a matching command") {
    nlohmann::json cfg = {{"blacklist", nlohmann::json::array({"reboot", "shutdown"})}};
    CHECK(is_command_blacklisted(cfg, "sudo reboot now"));
}

TEST_CASE("is_command_blacklisted returns false for a non-matching command") {
    nlohmann::json cfg = {{"blacklist", nlohmann::json::array({"reboot", "shutdown"})}};
    CHECK_FALSE(is_command_blacklisted(cfg, "ls -la"));
}

TEST_CASE("is_command_blacklisted returns false for an empty blacklist") {
    nlohmann::json cfg = {{"blacklist", nlohmann::json::array({})}};
    CHECK_FALSE(is_command_blacklisted(cfg, "reboot"));
}

TEST_CASE("is_command_blacklisted checks later entries after an earlier miss") {
    nlohmann::json cfg = {{"blacklist", nlohmann::json::array({"shutdown", "reboot"})}};
    CHECK(is_command_blacklisted(cfg, "sudo reboot"));
}

TEST_CASE("is_command_blacklisted is case sensitive") {
    // documents current behavior: no case-insensitive matching
    nlohmann::json cfg = {{"blacklist", nlohmann::json::array({"reboot"})}};
    CHECK_FALSE(is_command_blacklisted(cfg, "REBOOT"));
}

TEST_CASE("is_command_blacklisted matches substrings inside unrelated words") {
    // documents current behavior: "halt" also matches inside "asphalt"
    nlohmann::json cfg = {{"blacklist", nlohmann::json::array({"halt"})}};
    CHECK(is_command_blacklisted(cfg, "tar -xf asphalt.tar"));
}

TEST_CASE("is_valid_color_name accepts a lowercase hex code") {
    CHECK(is_valid_color_name("#ff8800"));
}

TEST_CASE("is_valid_color_name accepts an uppercase hex code") {
    CHECK(is_valid_color_name("#FF8800"));
}

TEST_CASE("is_valid_color_name rejects a hex code missing the '#'") {
    CHECK_FALSE(is_valid_color_name("ff8800"));
}

TEST_CASE("is_valid_color_name rejects a short hex code") {
    CHECK_FALSE(is_valid_color_name("#fff"));
}

TEST_CASE("is_valid_color_name rejects a too-long hex code") {
    CHECK_FALSE(is_valid_color_name("#ff8800ff"));
}

TEST_CASE("is_valid_color_name rejects non-hex characters") {
    CHECK_FALSE(is_valid_color_name("#gggggg"));
}

TEST_CASE("is_valid_color_name still accepts named colors") {
    CHECK(is_valid_color_name("cyan"));
}

TEST_CASE("is_valid_color_name rejects an unknown name") {
    CHECK_FALSE(is_valid_color_name("notacolor"));
}

TEST_CASE("color_from_string parses a hex code into the matching RGB color") {
    CHECK(color_from_string("#ff8800") == ftxui::Color::RGB(0xff, 0x88, 0x00));
}

TEST_CASE("color_from_string is case-insensitive for hex digits") {
    CHECK(color_from_string("#FF8800") == color_from_string("#ff8800"));
}

TEST_CASE("color_from_string falls back to white for an invalid color") {
    CHECK(color_from_string("notacolor") == ftxui::Color::White);
}

TEST_CASE("load_json returns nullopt for a missing file") {
    auto result = load_json((std::filesystem::temp_directory_path() / "lmcli_test_missing.json").string());
    CHECK_FALSE(result.has_value());
}

TEST_CASE("load_json returns nullopt for malformed JSON") {
    const auto path = (std::filesystem::temp_directory_path() / "lmcli_test_malformed.json").string();
    {
        std::ofstream file(path);
        file << "{ not valid json ";
    }

    auto result = load_json(path);
    CHECK_FALSE(result.has_value());

    std::filesystem::remove(path);
}

TEST_CASE("load_json returns nullopt for an empty JSON object") {
    // documents current behavior: {} parses fine but is treated as "empty" and rejected
    const auto path = (std::filesystem::temp_directory_path() / "lmcli_test_empty_object.json").string();
    {
        std::ofstream file(path);
        file << "{}";
    }

    auto result = load_json(path);
    CHECK_FALSE(result.has_value());

    std::filesystem::remove(path);
}

TEST_CASE("load_json parses a valid non-empty JSON file") {
    const auto path = (std::filesystem::temp_directory_path() / "lmcli_test_valid.json").string();
    {
        std::ofstream file(path);
        file << R"({"key": "value"})";
    }

    auto result = load_json(path);
    REQUIRE(result.has_value());
    CHECK((*result)["key"] == "value");

    std::filesystem::remove(path);
}

TEST_CASE("save_json then load_json round-trips the same data") {
    const auto path = (std::filesystem::temp_directory_path() / "lmcli_test_roundtrip.json").string();
    nlohmann::json data = {{"name", "tech"}, {"count", 3}};

    save_json(path, data);
    auto result = load_json(path);

    REQUIRE(result.has_value());
    CHECK(*result == data);

    std::filesystem::remove(path);
}
