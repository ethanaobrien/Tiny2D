# Browser support via Emscripten
find_package(Threads)

set(PLATFORM_INPUT       src-engine/input/emscripten.cpp)
set(PLATFORM_DEFS         "")
set(PLATFORM_FLAGS        -sUSE_SDL=2 -sUSE_SDL_TTF=2)
set(PLATFORM_LIBS         "")
list(APPEND PLATFORM_LINK_FLAGS -sUSE_SDL=2 -sUSE_SDL_TTF=2)

list(APPEND PLATFORM_LINK_FLAGS
    -sMODULARIZE=1
    -sEXPORT_NAME=GAME_2
    -sWASM=1
    -sNO_EXIT_RUNTIME=1
    -sALLOW_MEMORY_GROWTH=1
    -sEXPORTED_RUNTIME_METHODS=[callMain,registerPostMainLoop,FS,HEAPU8]
)

if(EXISTS ${CMAKE_SOURCE_DIR}/vendor/SuperShiny-0v0rG.ttf)
    list(APPEND PLATFORM_LINK_FLAGS --embed-file=${CMAKE_SOURCE_DIR}/vendor/SuperShiny-0v0rG.ttf@/vendor/SuperShiny-0v0rG.ttf)
    message(STATUS "Emscripten: embedding vendor/SuperShiny-0v0rG.ttf")
endif()
