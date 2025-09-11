#include <gegege/otsukimi/otsukimi.hpp>
#include <gegege/otsukimi/lua_app.hpp>
#include <vulkan/vulkan.hpp>

#include <windows.h>

#define GLAD_GL_IMPLEMENTATION
#include <gegege/otsukimi/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

int WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nShowCmd)
{
    gegege::otsukimi::LuaApp otsukimi;
    otsukimi.startup();
    otsukimi.run();
    otsukimi.shutdown();
    return 0;
}
