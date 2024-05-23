cmake_minimum_required(VERSION 3.29)

project(cursedui VERSION 1.0
        DESCRIPTION "UI-framework for terminal UI in C++ inspired by Android Views"
        LANGUAGES CXX)

include(FetchContent)
include(cmake/cursedui_component.cmake)
include(cmake/cursedui_tests.cmake)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG "v1.14.0"
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
)

option(CURSEDUI_BUILD_SHARED "Builds cursedui as a shared library" ON)
option(CURSEDUI_BUILD_TESTS "Builds tests" ON)
option(CURSEDUI_BUILD_EXAMPLES "Builds examples" ON)

set(CMAKE_CXX_FLAGS_DEBUG
        "-g -O0 -fasynchronous-unwind-tables -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE
        "-g0 -O3 -flto -fno-asynchronous-unwind-tables -ffunction-sections -fdata-sections -DNDEBUG")

if (CURSEDUI_BUILD_TESTS)
    enable_testing()
endif ()

add_subdirectory(base)
add_subdirectory(avada)
add_subdirectory(cursedui)
