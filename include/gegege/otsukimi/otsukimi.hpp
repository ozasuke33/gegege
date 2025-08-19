#pragma once

#include <gegege/lua_engine/lua_engine.hpp>

#include <filesystem>

namespace gegege::otsukimi {

struct Otsukimi {
    lua::LuaEngine luaEngine;

    void startup()
    {
        luaEngine.startup();
        luaEngine.openlibs();

        SDL_Init(SDL_INIT_VIDEO);

        std::filesystem::path path = SDL_GetBasePath();
        SDL_Log("Base Path: %s", path.generic_string().c_str());

        // set data/?.lua
        path.append("data");
        path.append("?.lua");
        auto pkgPath = gegege::lua::getLuaValueString(luaEngine.getTable("package", "path"));
        pkgPath.append(";");
        pkgPath.append(path.generic_string());

        // set data/?/init.lua
        path = SDL_GetBasePath();
        path.append("data");
        path.append("?");
        path.append("init.lua");
        pkgPath.append(";");
        pkgPath.append(path.generic_string());

        luaEngine.setTable("package", "path", gegege::lua::LuaString::make(pkgPath));

        path = SDL_GetBasePath();
        path.append("data");
        path.append("main.lua");

        luaEngine.executeFile(path.generic_string().c_str());
    }

    void shutdown()
    {
        SDL_Quit();

        luaEngine.shutdown();
    }
};

}
