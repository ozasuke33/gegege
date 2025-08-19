#pragma once

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include <SDL3/SDL.h>

#include <cassert>
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
    const LuaType mType = LuaType::eNil;
    const std::nullptr_t mValue = nullptr;
    static LuaNil make() { return LuaNil(); }

private:
    LuaNil() = default;
};

struct LuaBoolean final {
    const LuaType mType = LuaType::eBoolean;
    const bool mValue;
    static LuaBoolean make(const bool value) { return LuaBoolean(value); }

private:
    LuaBoolean(const bool value) : mValue(value) {}
};

struct LuaNumber final {
    const LuaType mType = LuaType::eNumber;
    const double mValue;
    static LuaNumber make(const double value) { return LuaNumber(value); }

private:
    LuaNumber(const double value) : mValue(value) {}
};

struct LuaString final {
    const LuaType mType = LuaType::eString;
    const std::string mValue;
    static LuaString make(const std::string& value) { return LuaString(value); }

private:
    LuaString(const std::string& value) : mValue(value) {}
};

using LuaValue = std::variant<LuaNil, LuaBoolean, LuaNumber, LuaString>;

inline LuaType getLuaType(const LuaValue& value)
{
    return std::visit(
        [](const auto& v) { return v.mType; },
        value);
}

inline std::string getLuaValueString(const LuaValue& value)
{
    switch (getLuaType(value))
    {
        case LuaType::eNil:
            return "nil";
        case LuaType::eBoolean:
            return std::get<LuaBoolean>(value).mValue
                       ? "true"
                       : "false";
        case LuaType::eNumber:
            return std::to_string(
                std::get<LuaNumber>(value).mValue);
        case LuaType::eString:
            return std::get<LuaString>(value).mValue;
    }
}

class LuaEngine {
    lua_State* mL;

public:
    void startup()
    {
        mL = luaL_newstate();
    }

    void shutdown()
    {
        lua_close(mL);
    }

    void openlibs()
    {
        luaL_openlibs(mL);
    }

    void pushValue(const LuaValue& value)
    {
        switch (getLuaType(value))
        {
            case LuaType::eNil:
                lua_pushnil(mL);
                break;
            case LuaType::eBoolean:
                lua_pushboolean(mL, std::get<LuaBoolean>(value).mValue ? 1 : 0);
                break;
            case LuaType::eNumber:
                lua_pushnumber(mL, std::get<LuaNumber>(value).mValue);
                break;
            case LuaType::eString:
                lua_pushstring(mL, std::get<LuaString>(value).mValue.c_str());
                break;
        }
    }

    LuaValue getValue(int index)
    {
        switch (lua_type(mL, index))
        {
            case LUA_TNIL:
                return LuaNil::make();
            case LUA_TBOOLEAN:
                return LuaBoolean::make(
                    lua_toboolean(mL, index) == 1);
            case LUA_TNUMBER:
                return LuaNumber::make(
                    (double)lua_tonumber(mL, index));
            case LUA_TSTRING:
                return LuaString::make(lua_tostring(mL, index));
            default:
                return LuaNil::make();
        }
    }

    LuaValue popValue()
    {
        auto value = getValue(-1);
        lua_pop(mL, 1);
        return value;
    }

    std::vector<LuaValue> popValues(int n)
    {
        std::vector<LuaValue> results;
        for (int i = n; i > 0; --i)
        {
            results.push_back(getValue(-i));
        }
        lua_pop(mL, n);
        return results;
    }

    std::string popString()
    {
        auto result = std::get<LuaString>(popValue());
        lua_pop(mL, 1);
        return result.mValue;
    }

    void execute(const std::string& code)
    {
        if (luaL_loadstring(mL, code.c_str()) != LUA_OK)
        {
            SDL_Log("Lua Engine: Failed to prepare script: %s", popString().c_str());
        }

        pcall();
    }

    void executeFile(const std::string& path)
    {
        if (luaL_loadfile(mL, path.c_str()) != LUA_OK)
        {
            SDL_Log("Lua Engine: Failed to prepare file: %s", popString().c_str());
        }

        pcall();
    }

    bool pcall(int nargs = 0, int nresult = 0)
    {
        if (lua_pcall(mL, nargs, nresult, 0) != LUA_OK)
        {
            SDL_Log("Lua Engine: Failed to execute Lua code: %s", popString().c_str());
            return false;
        }

        return true;
    }

    template <typename... Ts>
    LuaValue call(const std::string& function, const Ts&... params)
    {
        int type = lua_getglobal(mL, function.c_str());
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
        int stackSz = lua_gettop(mL);
        int type = lua_getglobal(mL, function.c_str());
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
            int nresults = lua_gettop(mL) - stackSz;
            return popValues(nresults);
        }
        return std::vector<LuaValue>();
    }

    LuaValue getGlobal(const std::string& name)
    {
        lua_getglobal(mL, name.c_str());
        return popValue();
    }

    void setGlobal(const std::string& name, const LuaValue& value)
    {
        pushValue(value);
        lua_setglobal(mL, name.c_str());
    }

    LuaValue getTable(const std::string& table, const std::string& key)
    {
        int type = lua_getglobal(mL, table.c_str());
        assert(type == LUA_TTABLE);
        lua_pushstring(mL, key.c_str());
        lua_gettable(mL, -2);
        auto value = popValue();
        lua_pop(mL, 1); // pop table
        return value;
    }

    void setTable(const std::string& table, const std::string& key, const LuaValue& value)
    {
        int type = lua_getglobal(mL, table.c_str());
        assert(type == LUA_TTABLE);
        lua_pushstring(mL, key.c_str());
        pushValue(value);
        lua_settable(mL, -3);
        lua_pop(mL, 1); // pop table
    }
};

} // namespace gegege::lua
