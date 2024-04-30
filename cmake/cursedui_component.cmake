
# Declare a component
function(cursedui_component)
    cmake_parse_arguments(
            CURSEDUI
            ""  # flags
            "NAME"  # single-argument
            "SOURCES"  # multi-arguments
            ${ARGN}
    )

    if (CURSEDUI_BUILD_SHARED)
        set(CONFIG_SHARED 1)
        set(LIB_TYPE SHARED)
    else ()
        set(CONFIG_SHARED 0)
        set(LIB_TYPE STATIC)
    endif ()


    string(TOUPPER ${CURSEDUI_NAME} CONFIG_COMPONENT_NAME)
    configure_file("${CMAKE_SOURCE_DIR}/cmake/component_config.hpp.in" "config.hpp")

    add_library(${CURSEDUI_NAME} ${LIB_TYPE} ${CURSEDUI_SOURCES})
    target_sources(${CURSEDUI_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/config.hpp")

    if (CURSEDUI_BUILD_SHARED)
        target_compile_options(${CURSEDUI_NAME} PRIVATE
                "-fvisibility=hidden"
                "-fvisibility-inlines-hidden"
        )
    endif ()

    target_include_directories(${CURSEDUI_NAME} PUBLIC ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
    add_library(cursedui::${CURSEDUI_NAME} ALIAS ${CURSEDUI_NAME})

endfunction()
