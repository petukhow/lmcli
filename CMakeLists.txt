cmake_minimum_required(VERSION 3.15)
project(lmcli VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL REQUIRED)

file(GLOB_RECURSE SOURCES 
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(lmcli ${SOURCES})

target_include_directories(lmcli PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(lmcli PRIVATE 
    CURL::libcurl
)

if (MSVC)
    target_compile_options(lmcli PRIVATE /W4)
else ()
    target_compile_options(lmcli PRIVATE -Wall -Wextra -Wpedantic)
endif ()

set_target_properties(lmcli PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS lmcli DESTINATION bin)
install(FILES data/providers.json DESTINATION /usr/share/lmcli)