cmake_minimum_required(VERSION 3.15)
project(lmcli)

set(CMAKE_CXX_STANDARD 17)

find_package(CURL REQUIRED)

add_executable(lmcli src/accounts.cpp src/anthropic.cpp src/config.cpp src/main.cpp
    src/openAICompatible.cpp src/providers.cpp src/selectAccount.cpp src/setupAccount.cpp
    src/startChat src/utils.cpp)
target_include_directories(lmcli PRIVATE include)
target_link_libraries(lmcli CURL::libcurl)