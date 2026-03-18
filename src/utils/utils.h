#pragma once
#include <string>

std::string get_config_path(const std::string& filename);

std::string get_config_dir();

void create_config_file_if_not_exists(const std::string& config_dir, const std::string& file_template);

std::string create_file_if_not_exists(const std::string& chats_dir, const std::string& file_template);

std::string get_system_data_path(const std::string& filename);

std::string get_chats_dir();

std::string get_chats_path(const std::string& filename);
