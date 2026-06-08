#pragma once
#include "message.h"
#include <vector>

void handle_tool_calls(const Message& output, std::vector<Message>& conversation);