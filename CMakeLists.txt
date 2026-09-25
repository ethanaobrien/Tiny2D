cmake_minimum_required(VERSION 3.16)
project(game LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =========================================================================
#  Platform selection
#
#  A "platform" is just a file in cmake/platforms/ that sets these variables:
#
#     PLATFORM_INPUT        the one input-backend .cpp for this platform
#     PLATFORM_DEFS         preprocessor defines        (list, may be empty)
#     PLATFORM_FLAGS        extra compile options       (list, may be empty)
#     PLATFORM_LIBS         libraries to link           (list, may be empty)
#     PLATFORM_LINK_FLAGS   extra linker flags          (list, may be empty)
#     PLATFORM_INCLUDE_DIRS extra include dirs          (list, may be empty)
#
#  Adding a platform is therefore: drop cmake/platforms/<name>.cmake, add <name>
#  to the STRINGS list below, and (only if it needs its own compiler) a toolchain
#  file. Nothing in the core build below ever changes.
# =========================================================================
set(PLATFORM "linux" CACHE STRING "Target platform")
set_property(CACHE PLATFORM PROPERTY STRINGS linux windows emscripten switch)

include(cmake/platforms/${PLATFORM}.cmake)

find_package(Threads)

option(DEBUG_STATS "Enable the debug stats overlay" OFF)

# --- The engine + rendering: a static lib, byte-for-byte identical per platform
add_library(engine STATIC
    src-engine/engine.cpp
    src-engine/gameobject.cpp
    src-engine/drawing.cpp
    src-engine/input/common.cpp
    ${PLATFORM_INPUT}
)

# --- Game logic (Game class + level loader). Also platform-agnostic.
add_library(gamelogic STATIC
    src/game/game.cpp
)

# --- The executable: core + entry point
add_executable(game
    src/main.cpp
)

# Shared include paths (the src/ vs src-engine/ split). PLATFORM_INCLUDE_DIRS
# lets a platform inject extra dirs (e.g. the emscripten SDL shim) untouched.
foreach(tgt engine gamelogic game)
    target_include_directories(${tgt} PUBLIC
        src-engine
        src-engine/input
        src
        src/game
        vendor               # nlohmann/json
        ${PLATFORM_INCLUDE_DIRS}
    )
    target_compile_definitions(${tgt} PUBLIC ${PLATFORM_DEFS}
        $<$<BOOL:${DEBUG_STATS}>:DEBUG_STATS>)
    target_compile_options(${tgt}    PUBLIC ${PLATFORM_FLAGS})
endforeach()

target_link_libraries(game PRIVATE gamelogic engine ${PLATFORM_LIBS})
if(Threads_FOUND)
    target_link_libraries(game PRIVATE Threads::Threads)
endif()

if(PLATFORM_LINK_FLAGS)
    target_link_options(game PRIVATE ${PLATFORM_LINK_FLAGS})
endif()

# Native -> game.bin (matches the old Makefile); emscripten -> game.js.
if(NOT PLATFORM STREQUAL "emscripten")
    set_target_properties(game PROPERTIES OUTPUT_NAME "game.bin")
else()
    set_target_properties(game PROPERTIES SUFFIX ".js" OUTPUT_NAME "game")
endif()

message(STATUS "Platform      : ${PLATFORM}")
message(STATUS "Input backend : ${PLATFORM_INPUT}")
message(STATUS "Debug stats   : ${DEBUG_STATS}")
