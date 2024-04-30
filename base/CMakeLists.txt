cursedui_component(
        NAME base
        SOURCES

        debug/debug.cc
        debug/debug.hpp
        debug/stack_trace.cc
        debug/stack_trace.hpp
        debug/tracing.cc
        debug/tracing.hpp

        enum_flags.hpp
        exception.cc
        exception.hpp
        macro.hpp
        nullable.hpp
        ref_ptr.cc
        ref_ptr.hpp
        run_loop.cc
        run_loop.hpp
        type_array.hpp
        util.hpp
        weak_ref.cc
        weak_ref.hpp
)

cursedui_tests(
        NAME base
        SOURCES
        ref_ptr_unittest.cc
        run_loop_unittest.cc
        weak_ref_unittest.cc
)