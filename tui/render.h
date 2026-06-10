#pragma once
#include <string>

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/screen.hpp>

using namespace ftxui;

void show_message(const std::string& msg, Color c = Color::Yellow);