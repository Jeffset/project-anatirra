# Project Anatirra

My public **C++** monorepo with with a variety of my personal bits of projects.

> [!TIP]
> The code here may be in various states of being (in)complete. Use with caution.

Though I strive to keep things well-tested and organized here, I have limited resources to do so.

Currently the project is using `C++20` *modulo* modules.

## library: avada
Avada is a lower-level library providing means to efficiently render contents
with ANSI terminal sequences. Designed to replace `ncurses`.

## library: cursedui
The high-level library implementing UI elements, based on `libavada`. 
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

TBD