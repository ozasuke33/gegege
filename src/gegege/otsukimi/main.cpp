#include <gegege/otsukimi/otsukimi.hpp>
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
    gegege::otsukimi::Otsukimi otsukimi;
    otsukimi.startup();
    otsukimi.run();
    otsukimi.shutdown();
    return 0;
}
