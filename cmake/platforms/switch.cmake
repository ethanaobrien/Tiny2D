# Nintendo Switch (libnx). TEMPLATE - unverified here.
#
# The Makefile already had a libnx branch; a real backend for it would live at
# src-engine/input/libnx.cpp. To wire it up:
#   1. write src-engine/input/libnx.cpp implementing the InputDriver interface
#      (AddListener/AddPressListener/SetupBackend/Tick/Dispatch, like sdl.cpp)
#   2. point PLATFORM_INPUT at it below and add the libnx SDK include/link.
find_package(Threads)

set(PLATFORM_INPUT       src-engine/input/libnx.cpp)
set(PLATFORM_DEFS        "__SWITCH__")
set(PLATFORM_FLAGS       "-I${LIBNX_SDK}/include")
set(PLATFORM_LIBS       ${LIBNX_SDK}/lib/libnx.a)
set(PLATFORM_LINK_FLAGS   "")
set(PLATFORM_INCLUDE_DIRS    "")
