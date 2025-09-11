#pragma once

#include "../lua_engine/lua_engine.hpp"
#include "gl.h"
#include "renderer.hpp"
#include "graphics.hpp"
#include "lua_graphics.hpp"
#include "lua_util.hpp"

#include <filesystem>

namespace gegege::otsukimi {

extern Renderer* gRenderer;

struct Otsukimi {
    lua::LuaEngine mLuaEngine;
    SDL_Window* mSdlWindow;
    SDL_GLContext mGlContext;
    bool mStopRendering = false;
    uint64_t mPrevTime;
    Renderer mRenderer;
    bool mFullscreen;

    virtual void startup();

    virtual void shutdown();

    virtual void run();

    virtual void onResized(int w, int h)
    {
    }

    virtual void onMousePressed(float x, float y, const std::vector<bool> button)
    {
    }

    virtual void onKeyPressed(const std::string& keyName, const std::string& scancodeName, bool isRepeat)
    {
    }

    virtual void onUpdate(float dt)
    {
    }
};

} // namespace gegege::otsukimi
