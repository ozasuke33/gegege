include(FetchContent)

FetchContent_Declare(SDL
    URL https://github.com/libsdl-org/SDL/archive/refs/tags/release-3.2.0.tar.gz
)
set(SDL_SHARED_DEFAULT OFF CACHE INTERNAL "")
set(SDL_SHARED OFF CACHE INTERNAL "")
set(SDL_STATIC ON CACHE INTERNAL "")
FetchContent_MakeAvailable(SDL)


FetchContent_Declare(VMA
    URL https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/archive/refs/tags/v3.2.0.tar.gz
)
FetchContent_MakeAvailable(VMA)

FetchContent_Declare(lua
    URL https://github.com/lua/lua/archive/refs/tags/v5.4.7.tar.gz
)
FetchContent_MakeAvailable(lua)
add_library(lua STATIC
    ${lua_SOURCE_DIR}/lzio.c
    ${lua_SOURCE_DIR}/lctype.c
    ${lua_SOURCE_DIR}/lopcodes.c
    ${lua_SOURCE_DIR}/lmem.c
    ${lua_SOURCE_DIR}/lundump.c
    ${lua_SOURCE_DIR}/ldump.c
    ${lua_SOURCE_DIR}/lstate.c
    ${lua_SOURCE_DIR}/lgc.c
    ${lua_SOURCE_DIR}/llex.c
    ${lua_SOURCE_DIR}/lcode.c
    ${lua_SOURCE_DIR}/lparser.c
    ${lua_SOURCE_DIR}/ldebug.c
    ${lua_SOURCE_DIR}/lfunc.c
    ${lua_SOURCE_DIR}/lobject.c
    ${lua_SOURCE_DIR}/ltm.c
    ${lua_SOURCE_DIR}/lstring.c
    ${lua_SOURCE_DIR}/ltable.c
    ${lua_SOURCE_DIR}/ldo.c
    ${lua_SOURCE_DIR}/lvm.c
    ${lua_SOURCE_DIR}/lapi.c

    ${lua_SOURCE_DIR}/lauxlib.c

    ${lua_SOURCE_DIR}/lbaselib.c
    ${lua_SOURCE_DIR}/lcorolib.c
    ${lua_SOURCE_DIR}/ldblib.c
    ${lua_SOURCE_DIR}/liolib.c
    ${lua_SOURCE_DIR}/lmathlib.c
    ${lua_SOURCE_DIR}/loadlib.c
    ${lua_SOURCE_DIR}/loslib.c
    ${lua_SOURCE_DIR}/lstrlib.c
    ${lua_SOURCE_DIR}/ltablib.c
    ${lua_SOURCE_DIR}/lutf8lib.c
    ${lua_SOURCE_DIR}/linit.c
)
target_include_directories(lua PUBLIC ${lua_SOURCE_DIR})
