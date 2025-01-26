#pragma once

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include <SDL3/SDL.h>

#include <string>
#include <variant>
#include <vector>

namespace gegege::lua {

enum class LuaType {
    eNil,
    eBoolean,
    eNumber,
    eString
};

struct LuaNil final {
    const LuaType type = LuaType::eNil;
    const std::nullptr_t value = nullptr;
    static LuaNil make() { return LuaNil(); }

private:
    LuaNil() = default;
};

struct LuaBoolean final {
    const LuaType type = LuaType::eBoolean;
    const bool value;
    static LuaBoolean make(const bool value) { return LuaBoolean(value); }

private:
    LuaBoolean(const bool value) : value(value) {}
};

struct LuaNumber final {
    const LuaType type = LuaType::eNumber;
    const double value;
    static LuaNumber make(const double value) { return LuaNumber(value); }

private:
    LuaNumber(const double value) : value(value) {}
};

struct LuaString final {
    const LuaType type = LuaType::eString;
    const std::string value;
    static LuaString make(const std::string& value) { return LuaString(value); }

private:
    LuaString(const std::string& value) : value(value) {}
};

using LuaValue = std::variant<LuaNil, LuaBoolean, LuaNumber, LuaString>;

inline LuaType getLuaType(const LuaValue& value)
{
    return std::visit(
        [](const auto& v) { return v.type; },
        value);
}

inline std::string getLuaValueString(const LuaValue& value)
{
    switch (getLuaType(value))
    {
        case LuaType::eNil:
            return "nil";
        case LuaType::eBoolean:
            return std::get<LuaBoolean>(value).value
                       ? "true"
                       : "false";
        case LuaType::eNumber:
            return std::to_string(
                std::get<LuaNumber>(value).value);
        case LuaType::eString:
            return std::get<LuaString>(value).value;
    }
}

class LuaEngine {
    lua_State* L;

public:
    void startup()
    {
        L = luaL_newstate();
    }

    void shutdown()
    {
        lua_close(L);
    }

    void openlibs()
    {
        luaL_openlibs(L);
    }

    void pushValue(const LuaValue& value)
    {
        switch (getLuaType(value))
        {
            case LuaType::eNil:
                lua_pushnil(L);
                break;
            case LuaType::eBoolean:
                lua_pushboolean(L, std::get<LuaBoolean>(value).value ? 1 : 0);
                break;
            case LuaType::eNumber:
                lua_pushnumber(L, std::get<LuaNumber>(value).value);
                break;
            case LuaType::eString:
                lua_pushstring(L, std::get<LuaString>(value).value.c_str());
                break;
        }
    }

    LuaValue getValue(int index)
    {
        switch (lua_type(L, index))
        {
            case LUA_TNIL:
                return LuaNil::make();
            case LUA_TBOOLEAN:
                return LuaBoolean::make(
                    lua_toboolean(L, index) == 1);
            case LUA_TNUMBER:
                return LuaNumber::make(
                    (double)lua_tonumber(L, index));
            case LUA_TSTRING:
                return LuaString::make(lua_tostring(L, index));
            default:
                return LuaNil::make();
        }
    }

    LuaValue popValue()
    {
        auto value = getValue(-1);
        lua_pop(L, 1);
        return value;
    }

    std::vector<LuaValue> popValues(int n)
    {
        std::vector<LuaValue> results;
        for (int i = n; i > 0; --i)
        {
            results.push_back(getValue(-i));
        }
        lua_pop(L, n);
        return results;
    }

    std::string popString()
    {
        auto result = std::get<LuaString>(popValue());
        lua_pop(L, 1);
        return result.value;
    }

    void execute(const std::string& code)
    {
        if (luaL_loadstring(L, code.c_str()) != LUA_OK)
        {
            SDL_Log("Lua Engine: Failed to prepare script: %s", popString().c_str());
        }

        pcall();
    }

    void executeFile(const std::string& path)
    {
        if (luaL_loadfile(L, path.c_str()) != LUA_OK)
        {
            SDL_Log("Lua Engine: Failed to prepare file: %s", popString().c_str());
        }

        pcall();
    }

    bool pcall(int nargs = 0, int nresult = 0)
    {
        if (lua_pcall(L, nargs, nresult, 0) != LUA_OK)
        {
            SDL_Log("Lua Engine: Failed to execute Lua code: %s", popString().c_str());
            return false;
        }

        return true;
    }

    template <typename... Ts>
    LuaValue call(const std::string& function, const Ts&... params)
    {
        int type = lua_getglobal(L, function.c_str());
        if (type != LUA_TFUNCTION)
        {
            abort();
        }
        for (auto param : std::initializer_list<LuaValue>{params...})
        {
            pushValue(param);
        }
        pcall(sizeof...(params), 1);
        return popValue();
    }

    template <typename... Ts>
    std::vector<LuaValue> vcall(
        const std::string& function, const Ts&... params)
    {
        int stackSz = lua_gettop(L);
        int type = lua_getglobal(L, function.c_str());
        if (type != LUA_TFUNCTION)
        {
            abort();
        }
        for (auto param :
             std::initializer_list<LuaValue>{params...})
        {
            pushValue(param);
        }
        if (pcall(sizeof...(params), LUA_MULTRET))
        {
            int nresults = lua_gettop(L) - stackSz;
            return popValues(nresults);
        }
        return std::vector<LuaValue>();
    }

    LuaValue getGlobal(const std::string& name)
    {
        lua_getglobal(L, name.c_str());
        return popValue();
    }

    void setGlobal(const std::string& name, const LuaValue& value)
    {
        pushValue(value);
        lua_setglobal(L, name.c_str());
    }
};

} // namespace gegege::lua
