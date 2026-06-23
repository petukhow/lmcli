#pragma once
#include "types/message.h"
#include <vector>

std::vector<Message> handle_tool_calls(const Message& output, const std::function<bool(const std::string&)>& confirm);