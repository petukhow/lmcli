cmake_minimum_required(VERSION 3.15)
project(lmcli)

set(CMAKE_CXX_STANDARD 17)

find_package(CURL REQUIRED)

add_executable(lmcli src/main.cpp src/openai-compatible.cpp src/utils.cpp
    src/config.cpp src/anthropic.cpp src/selectaccount.cpp src/accounts.cpp
    src/providers.cpp)
target_include_directories(lmcli PRIVATE include)
target_link_libraries(lmcli CURL::libcurl)