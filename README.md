# Overview

This is a small 2D game engine written in C++. It handles objects, movement, gravity, jumping, collisions, keyboard input, and drawing with SDL2. The camera can move around the world without changing the objects' positions.

The purpose of this project is to practice C++ classes, pointers, memory management, and STL containers while building an engine that can be used by a separate game.

The engine is in `src-engine/`. The game in `src/` was generated with AI to demonstrate the features of this engine and is separate from the work described here.

## Engine structure

- `engine.cpp` / `engine.h` - stores objects, runs physics, checks collisions, and manages the camera. A grid groups objects by horizontal position to reduce the number of collision checks.
- `gameobject.cpp` / `gameobject.h` - stores each object's position, appearance, and movement state. Movement is queued and then limited by collisions during physics.
- `drawing.cpp` / `drawing.h` - creates the SDL window and draws rectangles, borders, and text. Text textures are cached so they can be reused.
- `input/` - handles key combinations and callbacks, with support for emscripten and SDL.
- `collider.h`, `direction.h`, and `colors.h` - shared movement results, direction flags, and color values.

The engine uses variables and expressions to calculate movement and distances, conditionals to check collisions, and loops to update and draw objects. Functions divide this work between the `GameEngine`, `GameObject`, `DrawingEngine`, and `InputDriver` classes.

STL containers include `std::list` for objects, `std::vector` for the collision grid and queued events, and `std::unordered_map` for object lookups and cached text. `GameEngine` creates its drawing engine with `new` and frees it with `delete` in the destructor.

# Development Environment

The engine uses C++17 and CMake. The Linux build requires a C++ compiler, SDL2, and SDL2_ttf. Emscripten is used for browser support.

- SDL2 - window creation, rendering, and native input events
- SDL2_ttf - font loading and text rendering
- C++ standard library - containers, callbacks, shared pointers, and timing

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
