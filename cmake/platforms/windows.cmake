# Native Windows via MinGW-w64. Input backend: SDL2 (same as Linux).
#
# TEMPLATE - unverified here (no MinGW toolchain in the sandbox). Configure with:
#   cmake -G "MinGW Makefiles" -DPLATFORM=windows -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++
# and make sure SDL2 / SDL2_ttf are installed for the MinGW triplet.
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)

set(PLATFORM_INPUT       src-engine/input/sdl.cpp)
set(PLATFORM_DEFS         "")
set(PLATFORM_FLAGS        "")
set(PLATFORM_LIBS       SDL2::SDL2 SDL2_ttf::SDL2_ttf)
# SDL2 on Windows supplies SDL_main; the entry point is main.cpp's WinMain block.
set(PLATFORM_LINK_FLAGS   "")
set(PLATFORM_INCLUDE_DIRS  "")
