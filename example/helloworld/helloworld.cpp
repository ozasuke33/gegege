#include <gegege/lua_engine/lua_engine.hpp>
#include <gegege/vulkan_engine/vulkan_engine.hpp>

int main()
{
    using namespace gegege::lua;
    LuaEngine lua_engine{};
    lua_engine.startup();
    lua_engine.openlibs();
    lua_engine.execute(R"(
engine = 'opengl'
function greetings(...)
    local result = 'Hello'
    for i, v in ipairs{...} do
        result = result .. ' ' .. v .. ','
    end
    return result
end)");
    auto result = lua_engine.call("greetings", LuaString::make("C++"), LuaString::make("Lua"));
    SDL_Log("%s", getLuaValueString(result).c_str());
    SDL_Log("engine = %s", getLuaValueString(lua_engine.getGlobal("engine")).c_str());
    lua_engine.setGlobal("engine", LuaString::make("vulkan"));
    SDL_Log("engine = %s", getLuaValueString(lua_engine.getGlobal("engine")).c_str());
    lua_engine.shutdown();

    gegege::vulkan::VulkanEngine vulkan_engine{};
    vulkan_engine.startup();
    vulkan_engine.run();
    vulkan_engine.shutdown();
    return 0;
}
