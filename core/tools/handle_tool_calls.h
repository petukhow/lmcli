#pragma once
#include "types/message.h"
#include <vector>

void handle_tool_calls(const Message& output, std::vector<Message>& conversation, const std::function<bool(const std::string&)>& confirm);