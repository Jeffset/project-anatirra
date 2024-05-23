
cursedui_component(
        NAME cursedui
        SOURCES
        canvas.cc
        canvas.hpp
        dim.hpp
        drawable.hpp
        region.cc
        drawable.cc
        dim.cc
        region.hpp
        view.cc
        view.hpp
        view_data.cc
        view_data.hpp
        view_group.cc
        view_group.hpp
        view_specs.cc
        view_specs.hpp
        view_tree_host.cc
        view_tree_host.hpp

        views/frame_layout.cc
        views/frame_layout.hpp
        views/linear_layout.cc
        views/linear_layout.hpp
        views/scroll_view.hpp
        views/scroll_view.cc
        views/text_view.hpp
        views/text_view.cc
        views/button.hpp
        views/button.cc

        animation/animation.cc
        animation/animation.hpp
        animation/animation_host.cc
        animation/animation_host.hpp
        animation/animations.cc
        animation/animations.hpp
)

target_link_libraries(cursedui PUBLIC base avada)

if (CURSEDUI_BUILD_EXAMPLES)
    add_executable(cursedui_example main.cc)
    target_link_libraries(cursedui_example PRIVATE cursedui base)
endif ()

cursedui_tests(
        NAME cursedui
        SOURCES
        test/common_layout_unittest.cc
        test/frame_layout_unittest.cc
        test/region_unittest.cc
        test/test_harness.hpp
)