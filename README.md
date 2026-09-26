# Overview

This is a small 2D game engine written in C++. It handles objects, movement, gravity, jumping, collisions, keyboard input, and drawing with SDL2.

The purpose of this project is to practice C++ classes, pointers, memory management, and STL containers while building an engine that can be used by a separate game.

The engine is in `src-engine/`. The game in `src/` is a very basic example of what the game can look like.

# Development Environment

The engine uses C++17 and CMake. The Linux build requires a C++ compiler, SDL2, and SDL2_ttf.

- SDL2 - window creation, rendering, and native input events
- SDL2_ttf - font loading and text rendering
- C++ standard library - containers, callbacks, shared pointers, and timing
- Emscripten - used for the browser build

## Build and run on Linux

With everything installed, run this from the repository root:

```sh
cmake -S . -B build
cmake --build build
./build/game.bin
```

The demo uses Left/Right to move, Up to jump, and Down to restart. Close the window to quit. Its purpose here is to show the engine's movement, collision, camera, and drawing features.

# Useful Websites

- [C++ reference](https://en.cppreference.com/w/) - language syntax and STL containers
- [SDL2 documentation](https://wiki.libsdl.org/SDL2/FrontPage) - windows, rendering, and input events
- [SDL2_ttf documentation](https://wiki.libsdl.org/SDL2_ttf/FrontPage) - fonts and text rendering
- [CMake tutorial](https://cmake.org/cmake/help/latest/guide/tutorial/index.html) - configuring and building C++ projects
- [Emscripten documentation](https://emscripten.org/docs/) - compiling C++ for the browser

# Future Work

- Improve collision grid handling for negative coordinates and queries across larger distances.
- Add tests for jumping, pushing objects, and removing objects during collision callbacks.
- Add support for rendering triangles
