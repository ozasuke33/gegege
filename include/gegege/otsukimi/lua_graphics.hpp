#pragma once

#include "../lua_engine/lua_engine.hpp"
#include "graphics.hpp"

namespace gegege::otsukimi {

inline int lua_textureFind(lua_State* L)
{
    lua::LuaEngine lua;
    lua.mL = L;
    lua::LuaValue path = lua.popValue();
    Texture* tex = textureFind(lua::getLuaValueString(path));
    lua_pushlightuserdata(L, tex);
    return 1;
}

inline int lua_getTextureWidth(lua_State* L)
{
    lua::LuaEngine lua;
    lua.mL = L;
    Texture* tex = (Texture*)lua_touserdata(L, -1);
    lua.pushValue(lua::LuaNumber::make(getTextureWidth(tex)));
    return 1;
}

inline int lua_getTextureHeight(lua_State* L)
{
    lua::LuaEngine lua;
    lua.mL = L;
    Texture* tex = (Texture*)lua_touserdata(L, -1);
    lua.pushValue(lua::LuaNumber::make(getTextureHeight(tex)));
    return 1;
}

inline int lua_drawTexture(lua_State* L)
{
    lua::LuaEngine lua;
    lua.mL = L;

    lua::LuaValue a = lua.popValue();
    lua::LuaValue b = lua.popValue();
    lua::LuaValue g = lua.popValue();
    lua::LuaValue r = lua.popValue();

    lua::LuaValue dy = lua.popValue();
    lua::LuaValue dx = lua.popValue();

    lua::LuaValue angle = lua.popValue();

    lua::LuaValue scaleX = lua.popValue();
    lua::LuaValue scaleY = lua.popValue();

    lua::LuaValue sh = lua.popValue();
    lua::LuaValue sw = lua.popValue();
    lua::LuaValue sy = lua.popValue();
    lua::LuaValue sx = lua.popValue();

    Texture* tex = (Texture*)lua_touserdata(L, -1);

    drawTexture(tex,
                std::get<lua::LuaNumber>(sx).mValue, std::get<lua::LuaNumber>(sy).mValue, std::get<lua::LuaNumber>(sw).mValue, std::get<lua::LuaNumber>(sh).mValue,
                std::get<lua::LuaNumber>(scaleX).mValue, std::get<lua::LuaNumber>(scaleY).mValue,
                std::get<lua::LuaNumber>(angle).mValue,
                std::get<lua::LuaNumber>(dx).mValue, std::get<lua::LuaNumber>(dy).mValue,
                std::get<lua::LuaNumber>(r).mValue, std::get<lua::LuaNumber>(g).mValue, std::get<lua::LuaNumber>(b).mValue, std::get<lua::LuaNumber>(a).mValue);
    return 0;
}

inline int lua_fontFind(lua_State* L)
{
    lua::LuaEngine lua;
    lua.mL = L;
    lua::LuaValue ptSize = lua.popValue();
    lua::LuaValue path = lua.popValue();
    TTF_Font* font = fontFind(lua::getLuaValueString(path), std::get<lua::LuaNumber>(ptSize).mValue);
    lua_pushlightuserdata(L, font);
    return 1;
}

inline int lua_drawText(lua_State* L)
{
    lua::LuaEngine lua;
    lua.mL = L;
    lua::LuaValue a = lua.popValue();
    lua::LuaValue b = lua.popValue();
    lua::LuaValue g = lua.popValue();
    lua::LuaValue r = lua.popValue();
    lua::LuaValue text = lua.popValue();
    lua::LuaValue y = lua.popValue();
    lua::LuaValue x = lua.popValue();
    TTF_Font* font = (TTF_Font*)lua_touserdata(L, -1);
    drawText(font, std::get<lua::LuaNumber>(x).mValue, std::get<lua::LuaNumber>(y).mValue,
             lua::getLuaValueString(text),
             std::get<lua::LuaNumber>(r).mValue, std::get<lua::LuaNumber>(g).mValue, std::get<lua::LuaNumber>(b).mValue, std::get<lua::LuaNumber>(a).mValue);
    return 0;
}

inline int lua_setFontOutline(lua_State* L)
{
    lua::LuaEngine lua;
    lua.mL = L;
    lua::LuaValue outlineSize = lua.popValue();
    TTF_Font* font = (TTF_Font*)lua_touserdata(L, -1);
    setFontOutline(font, std::get<lua::LuaNumber>(outlineSize).mValue);
    return 0;
}

} // namespace gegege::otsukimi
