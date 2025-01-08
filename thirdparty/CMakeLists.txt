include(FetchContent)

FetchContent_Declare(SDL
    URL https://github.com/libsdl-org/SDL/archive/refs/tags/preview-3.1.8.tar.gz
)
set(SDL_SHARED_DEFAULT OFF CACHE INTERNAL "")
set(SDL_SHARED OFF CACHE INTERNAL "")
set(SDL_STATIC ON CACHE INTERNAL "")
FetchContent_MakeAvailable(SDL)
