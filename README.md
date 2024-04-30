# CURSEDUI

A library implementing terminal UI written in C++20

### avada
Avada is a lower-level library providing means to efficiently render contents
with ANSI terminal sequences.

### cursedui
The high-level library implementing UI elements, based on libavada. 
Layout routines and object model are inspired by Android Views.

## Building & Usage

The project uses CMake as its build system.

Build the library and run tests:
```bash
mkdir build && cd build  # Create a build dir
cmake .. && make -j      # Generate and build
ctest                    # Run all tests
```

Run the examples:
```bash
avada/avada_example 2>/dev/null         # Raw painting example
cursedui/cursedui_example 2>/dev/null   # UI example
```

To use the library in the project: