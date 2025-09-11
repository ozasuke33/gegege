#pragma once

#include "../lua_engine/lua_engine.hpp"
#include "util.hpp"

namespace gegege::otsukimi {

inline int lua_getMouseCoordinateToScreenCoordinateX(lua_State* L)
{
    lua::LuaEngine lua;
    lua.mL = L;
    lua::LuaValue mouseX = lua.popValue();

    lua.pushValue(lua::LuaNumber::make(getMouseCoordinateToScreenCoordinateX(std::get<lua::LuaNumber>(mouseX).mValue)));

    return 1;
}

inline int lua_getMouseCoordinateToScreenCoordinateY(lua_State* L)
{
    lua::LuaEngine lua;
    lua.mL = L;
    lua::LuaValue mouseY = lua.popValue();

    lua.pushValue(lua::LuaNumber::make(getMouseCoordinateToScreenCoordinateY(std::get<lua::LuaNumber>(mouseY).mValue)));

    return 1;
}

} // namespace gegege::otsukimi
