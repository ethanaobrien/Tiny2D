# CMake toolchain file for the Emscripten target.
#
# Usage:
#   cmake -DPLATFORM=emscripten -DCMAKE_TOOLCHAIN_FILE=cmake/emscripten-toolchain.cmake
#
# Per-platform -s flags are NOT here; they live in cmake/platforms/emscripten.cmake
# so the toolchain stays generic.
set(CMAKE_SYSTEM_NAME Emscripten)

set(CMAKE_C_COMPILER emcc)
set(CMAKE_CXX_COMPILER em++)

set(CMAKE_EXE_LINKER_FLAGS "-sMODULARIZE=1")
