#include "../../../include/gegege/otsukimi/otsukimi.hpp"
#include "../../../include/gegege/otsukimi/lua_app.hpp"

#include <windows.h>

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
