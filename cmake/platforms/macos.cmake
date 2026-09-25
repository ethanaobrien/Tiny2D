# Native macOS. Same Unix/SDL layout as linux: poll the SDL2 event queue.
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)

set(PLATFORM_INPUT       src-engine/input/sdl.cpp)
set(PLATFORM_DEFS         "")
set(PLATFORM_FLAGS        "")
set(PLATFORM_LIBS       SDL2::SDL2 SDL2_ttf::SDL2_ttf)
set(PLATFORM_LINK_FLAGS   "")
# Homebrew's SDL2::SDL2 target only adds .../include/SDL2 to the include path,
# but the engine includes <SDL2/SDL.h>, so add the parent dir too.
set(PLATFORM_INCLUDE_DIRS  ${SDL2_INCLUDE_DIR}/..)
