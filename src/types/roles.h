#pragma once
#include <string>
#include "json.hpp"

class Role {
private:
    std::string value;

public:
    Role(const std::string& value)
        : value(value) {}

    Role() : value("") {}

    bool operator==(const Role& other) const {
        return value == other.value;
    }

    static Role Assistant;
    static Role User;
    static Role Tool;
    static Role System;

    std::string get_value() const {
        return value;
    }
};

inline void to_json(nlohmann::json& j, const Role& r) {
    j = r.get_value();
}

inline void from_json(const nlohmann::json& j, Role& r) {
    r = Role(j.get<std::string>());
}