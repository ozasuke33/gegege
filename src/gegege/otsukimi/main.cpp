#include <gegege/otsukimi/otsukimi.hpp>
#include <vulkan/vulkan.hpp>

#include <windows.h>

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
