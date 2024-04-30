
cursedui_component(
        NAME avada

        SOURCES
        avada.cc
        avada.hpp
        buffer.hpp
        buffer.cc
        color.cc
        color.hpp
        input.cc
        input.hpp
        write.cc
        write.hpp
)

target_link_libraries(avada PUBLIC base)

if (CURSEDUI_BUILD_EXAMPLES)
    add_executable(avada_example
            demo.cc
    )
    target_link_libraries(avada_example avada)
endif ()
