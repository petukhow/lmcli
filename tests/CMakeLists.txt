include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.13.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(tests
    main.cpp
    ${CMAKE_SOURCE_DIR}/src/providers/anthropic.cpp
    ${CMAKE_SOURCE_DIR}/src/providers/open_ai_compatible.cpp
    ${CMAKE_SOURCE_DIR}/src/providers/google.cpp
    ${CMAKE_SOURCE_DIR}/src/providers/provider.cpp
    ${CMAKE_SOURCE_DIR}/src/streaming.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/http_utils.cpp
)

target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain CURL::libcurl)