#include "tools.h"
#include <sstream>
#include <iostream>
#include <string>
#include <fstream>

std::string read_file(const std::string filename) {
    std::ifstream file(filename); 

    if (!file.is_open()) {
        std::cerr << "Error: Could not open the file." << std::endl;
        return "";
    }

    std::ostringstream ss;
    ss << file.rdbuf();

    return ss.str();
}
