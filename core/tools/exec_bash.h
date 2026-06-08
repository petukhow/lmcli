#pragma once
#include <string>
#include <functional>

std::string exec_bash(const std::string& cmd, const std::function<bool(const std::string&)>& callback);