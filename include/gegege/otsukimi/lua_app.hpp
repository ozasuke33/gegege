#pragma once

#include "otsukimi.hpp"

namespace gegege::otsukimi {

int lua_myRequire(lua_State* L)
{
    lua::LuaEngine lua;
    lua.mL = L;
    std::string modname = lua.popString();

    lua_getglobal(lua.mL, "package");
    lua_getfield(lua.mL, -1, "loaded");
    lua_remove(lua.mL, -2);
    lua_getfield(lua.mL, -1, modname.c_str());
    lua_remove(lua.mL, -2);
    int type = lua_type(lua.mL, -1);
    if (type == LUA_TTABLE)
    {
        return 1;
    }
    else if (type == LUA_TBOOLEAN)
    {
        return 1;
    }
    else if (type == LUA_TNIL)
    {
        lua_pop(lua.mL, 1);

        std::filesystem::path path = SDL_GetBasePath();
        // set data/?.lua
        path.append("data");
        path.append(modname + ".lua");
        const char* code = (const char*)SDL_LoadFile(path.generic_string().c_str(), NULL);

        if (!code)
        {
            path = SDL_GetBasePath();
            path.append("data");
            path.append(modname);
            path.append("init.lua");
            code = (const char*)SDL_LoadFile(path.generic_string().c_str(), NULL);
            if (!code)
            {
                luaL_error(lua.mL, "Lua Engine: Failed to prepare file: %s", modname.c_str());
                return 0;
            }
        }

        if (luaL_loadstring(lua.mL, code) != LUA_OK)
        {
            SDL_Log("Lua Engine: Failed to prepare script: %s", lua.popString().c_str());
        }
        lua.pcall(0, 1);

        if (lua_type(lua.mL, -1) != LUA_TNIL)
        {
            lua_getglobal(lua.mL, "package");
            lua_getfield(lua.mL, -1, "loaded");
            lua_remove(lua.mL, -2);
            lua_pushstring(lua.mL, modname.c_str());
            lua_pushvalue(lua.mL, -3);
            lua_settable(lua.mL, -3);
            lua_pop(lua.mL, 1);
        }
        else
        {
            lua_getglobal(lua.mL, "package");
            lua_getfield(lua.mL, -1, "loaded");
            lua_remove(lua.mL, -2);
            lua_pushstring(lua.mL, modname.c_str());
            lua_pushboolean(lua.mL, true);
            lua_settable(lua.mL, -3);
            lua_pop(lua.mL, 1);
            lua_pushboolean(lua.mL, true);
        }
    }

    return 1;
}

struct LuaApp : Otsukimi {
    void startup() override
    {
        Otsukimi::startup();

        lua_register(mLuaEngine.mL, "require", lua_myRequire);
        lua_register(mLuaEngine.mL, "getMouseCoordinateToScreenCoordinateX", lua_getMouseCoordinateToScreenCoordinateX);
        lua_register(mLuaEngine.mL, "getMouseCoordinateToScreenCoordinateY", lua_getMouseCoordinateToScreenCoordinateY);
        lua_register(mLuaEngine.mL, "setOffscreenWidth", lua_setOffscreenWidth);
        lua_register(mLuaEngine.mL, "setOffscreenHeight", lua_setOffscreenHeight);
        lua_register(mLuaEngine.mL, "textureFind", lua_textureFind);
        lua_register(mLuaEngine.mL, "getTextureWidth", lua_getTextureWidth);
        lua_register(mLuaEngine.mL, "getTextureHeight", lua_getTextureHeight);
        lua_register(mLuaEngine.mL, "drawTexture", lua_drawTexture);
        lua_register(mLuaEngine.mL, "fontFind", lua_fontFind);
        lua_register(mLuaEngine.mL, "drawText", lua_drawText);
        lua_register(mLuaEngine.mL, "setFontOutline", lua_setFontOutline);

        std::filesystem::path path = SDL_GetBasePath();
        SDL_Log("Base Path: %s", path.generic_string().c_str());

        // set data/?.lua
        path.append("data");
        path.append("?.lua");
        auto pkgPath = gegege::lua::getLuaValueString(mLuaEngine.getTable("package", "path"));
        pkgPath.append(";");
        pkgPath.append(path.generic_string());

        // set data/?/init.lua
        path = SDL_GetBasePath();
        path.append("data");
        path.append("?");
        path.append("init.lua");
        pkgPath.append(";");
        pkgPath.append(path.generic_string());

        mLuaEngine.setTable("package", "path", gegege::lua::LuaString::make(pkgPath));

        path = SDL_GetBasePath();
        path.append("data");
        path.append("main.lua");

        mLuaEngine.executeFile(path.generic_string().c_str());
    }

    void shutdown() override
    {
        mLuaEngine.shutdown();
    }

    void onLoad() override
    {
        int type = lua_getglobal(mLuaEngine.mL, "onLoad");
        mLuaEngine.popValue();
        if (type == LUA_TFUNCTION)
        {
            mLuaEngine.call("onLoad");
        }
    }

    void onResized(int w, int h) override
    {
        int type = lua_getglobal(mLuaEngine.mL, "onResized");
        mLuaEngine.popValue();
        if (type == LUA_TFUNCTION)
        {
            mLuaEngine.call("onResized", lua::LuaNumber::make(w), lua::LuaNumber::make(h));
        }
    }

    void onMousePressed(float x, float y, const std::vector<bool> button) override
    {
        int type = lua_getglobal(mLuaEngine.mL, "onMousePressed");
        if (type == LUA_TFUNCTION)
        {
            mLuaEngine.pushValue(lua::LuaNumber::make(x));
            mLuaEngine.pushValue(lua::LuaNumber::make(y));
            lua_newtable(mLuaEngine.mL);
            lua_pushboolean(mLuaEngine.mL, button[0]);
            lua_rawseti(mLuaEngine.mL, -2, 1);
            lua_pushboolean(mLuaEngine.mL, button[1]);
            lua_rawseti(mLuaEngine.mL, -2, 2);
            lua_pushboolean(mLuaEngine.mL, button[2]);
            lua_rawseti(mLuaEngine.mL, -2, 3);
            lua_pushboolean(mLuaEngine.mL, button[3]);
            lua_rawseti(mLuaEngine.mL, -2, 4);
            lua_pushboolean(mLuaEngine.mL, button[4]);
            lua_rawseti(mLuaEngine.mL, -2, 5);
            mLuaEngine.pcall(3, 1);
        }
        lua_pop(mLuaEngine.mL, 1);
    }

    void onKeyPressed(const std::string& keyName, const std::string& scancodeName, bool isRepeat) override
    {
        int type = lua_getglobal(mLuaEngine.mL, "onKeyPressed");
        mLuaEngine.popValue();
        if (type == LUA_TFUNCTION)
        {
            mLuaEngine.call("onKeyPressed", lua::LuaString::make(keyName), lua::LuaString::make(scancodeName), lua::LuaBoolean::make(isRepeat));
        }
    }

    void onUpdate(float dt) override
    {
        int type = lua_getglobal(mLuaEngine.mL, "onUpdate");
        mLuaEngine.popValue();
        if (type == LUA_TFUNCTION)
        {
            mLuaEngine.call("onUpdate", gegege::lua::LuaNumber::make(dt));
        }
    }
};

} // namespace gegege::otsukimi
