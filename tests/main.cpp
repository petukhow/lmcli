#include <catch2/catch_test_macros.hpp>
#include <optional>
#include "anthropic.h"
#include "google.h"
#include "open_ai_compatible.h"
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
    CHECK(result == "");
}

TEST_CASE("Google extract_delta with empty response") {
    auto google = Google("", "", "", "", 0, 0);
    nlohmann::json response = {};

    auto result = google.extract_delta(response);
    CHECK(result == "");
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
    CHECK(result == "");
}