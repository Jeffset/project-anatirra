
# Declare a tests target for a component
function(cursedui_tests)
    cmake_parse_arguments(
            CURSEDUI
            ""  # flags
            "NAME"  # single-argument
            "SOURCES"  # multi-arguments
            ${ARGN}
    )

    if (CURSEDUI_BUILD_TESTS)
        enable_testing()

        include(GoogleTest)
        FetchContent_MakeAvailable(googletest)

        add_executable(${CURSEDUI_NAME}_tests ${CURSEDUI_SOURCES})
        target_link_libraries(${CURSEDUI_NAME}_tests ${CURSEDUI_NAME} GTest::gtest_main GTest::gmock)
        gtest_discover_tests(${CURSEDUI_NAME}_tests)
    endif ()
endfunction()