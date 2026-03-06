#pragma once
#include <memory>
#include "provider.h"
#include "json.hpp"

std::unique_ptr<Provider> select_account(const nlohmann::json& accounts, const nlohmann::json& config);