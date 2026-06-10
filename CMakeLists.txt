cmake_minimum_required(VERSION 3.15)
project(lmcli VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(GNUInstallDirs)
include(FetchContent)
enable_language(C)

find_package(CURL REQUIRED)

FetchContent_Declare(ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
    GIT_TAG v6.1.9
)
FetchContent_MakeAvailable(ftxui)

file(GLOB_RECURSE CORE_SOURCES "${CMAKE_SOURCE_DIR}/core/*.cpp")
file(GLOB_RECURSE CLI_SOURCES "${CMAKE_SOURCE_DIR}/cli/*.cpp")
file(GLOB_RECURSE TUI_SOURCES "${CMAKE_SOURCE_DIR}/tui/*.cpp")

add_executable(lmcli
    ${CORE_SOURCES}
    ${CLI_SOURCES}
    ${TUI_SOURCES}
    ${CMAKE_SOURCE_DIR}/vendor/linenoise.c
)

add_subdirectory(tests)

target_include_directories(lmcli PRIVATE
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/cli
    ${CMAKE_SOURCE_DIR}/tui
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/vendor
)

target_link_libraries(lmcli PRIVATE
    CURL::libcurl
    ftxui::component
    ftxui::dom
    ftxui::screen   
)

if (MSVC)
    target_compile_options(lmcli PRIVATE /W4)
else ()
    target_compile_options(lmcli PRIVATE -Wall -Wextra -Wpedantic)
endif ()

set_target_properties(lmcli PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_source_files_properties(
    ${CMAKE_SOURCE_DIR}/vendor/linenoise.c
    PROPERTIES LANGUAGE C
)

install(TARGETS lmcli RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES data/providers.json DESTINATION ${CMAKE_INSTALL_DATADIR}/lmcli)
install(FILES data/tools.json DESTINATION ${CMAKE_INSTALL_DATADIR}/lmcli)