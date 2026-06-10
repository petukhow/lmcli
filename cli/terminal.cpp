#include "terminal.h"
#include <cstddef>
#include <iostream>
#include "ansi_codes.h"

void clear_lines(size_t size) {
    size_t i = 0;
    while (i < size) {
        std::cout << ansi::UP;
        std::cout << ansi::CLEAR;
        ++i;
    }
}