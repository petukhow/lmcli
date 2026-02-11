#pragma once
#include <iostream>
#include <memory>
#include "provider.h"
#include "json.hpp"

using json = nlohmann::json;

std::unique_ptr<Provider> selectProvider(const json& config);