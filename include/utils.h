#pragma once
#include "message.h"
#include <vector>

bool limitExceeded(const std::vector<Message>& conversation, size_t limit);

std::string getConfigPath(const std::string& filename);

size_t curlWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata);
