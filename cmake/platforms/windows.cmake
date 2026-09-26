# Windows via MinGW-w64
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)

set(PLATFORM_INPUT       src-engine/input/sdl.cpp)
set(PLATFORM_DEFS         "")
set(PLATFORM_FLAGS        "")
set(PLATFORM_LIBS       SDL2::SDL2 SDL2_ttf::SDL2_ttf)
set(PLATFORM_LINK_FLAGS   "")
set(PLATFORM_INCLUDE_DIRS  "")
