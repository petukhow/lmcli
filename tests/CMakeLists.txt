include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.13.0
)
FetchContent_MakeAvailable(Catch2)

file(GLOB_RECURSE CORE_SOURCES "${CMAKE_SOURCE_DIR}/core/*.cpp")

add_executable(tests
    main.cpp
    ${CORE_SOURCES}
    ${CMAKE_SOURCE_DIR}/vendor/linenoise.c
)

target_include_directories(tests PRIVATE
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/cli
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/vendor
)

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain CURL::libcurl)

set_source_files_properties(
    ${CMAKE_SOURCE_DIR}/vendor/linenoise.c
    PROPERTIES LANGUAGE C
)