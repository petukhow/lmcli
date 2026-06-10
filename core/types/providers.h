#pragma once
#include <string>
#include "json.hpp"

class Providers {
private:
    std::string value;

public:
    Providers(const std::string& value)
        : value(value) {}

    Providers() : value("") {}

    bool operator==(const Providers& other) const {
        return value == other.value;
    }

    static Providers Anthropic;
    static Providers OpenAICompatible;
    static Providers Google;

    std::string get_value() const {
        return value;
    }
};

inline void to_json(nlohmann::json& j, const Providers& r) {
    j = r.get_value();
}

inline void from_json(const nlohmann::json& j, Providers& r) {
    r = Providers(j.get<std::string>());
}