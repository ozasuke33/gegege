#pragma once

#include <gegege/lua_engine/lua_engine.hpp>
#include <gegege/otsukimi/gl.h>
#include <gegege/otsukimi/renderer.hpp>

#include <filesystem>

namespace gegege::otsukimi {

Renderer* gRenderer;

int textureFind(lua_State* L)
{
    lua::LuaEngine lua;
    lua.mL = L;
    lua::LuaValue path = lua.popValue();
    Texture* tex = gRenderer->textureFind(lua::getLuaValueString(path));
    lua_pushlightuserdata(L, tex);
    return 1;
}

int drawTexture(lua_State* L)
{
    lua::LuaEngine lua;
    lua.mL = L;
    lua::LuaValue y = lua.popValue();
    lua::LuaValue x = lua.popValue();
    Texture* tex = (Texture*)lua_touserdata(L, -1);
    gRenderer->drawTexture(tex, std::get<lua::LuaNumber>(x).mValue, std::get<lua::LuaNumber>(y).mValue);
    return 0;
}

int fontFind(lua_State* L)
{
    lua::LuaEngine lua;
    lua.mL = L;
    lua::LuaValue ptSize = lua.popValue();
    lua::LuaValue path = lua.popValue();
    TTF_Font* font = gRenderer->fontFind(lua::getLuaValueString(path), std::get<lua::LuaNumber>(ptSize).mValue);
    lua_pushlightuserdata(L, font);
    return 1;
}

int drawText(lua_State* L)
{
    lua::LuaEngine lua;
    lua.mL = L;
    lua::LuaValue text = lua.popValue();
    lua::LuaValue y = lua.popValue();
    lua::LuaValue x = lua.popValue();
    TTF_Font* font = (TTF_Font*)lua_touserdata(L, -1);
    gRenderer->drawText(font, std::get<lua::LuaNumber>(x).mValue, std::get<lua::LuaNumber>(y).mValue, lua::getLuaValueString(text));
    return 0;
}

struct Otsukimi {
    lua::LuaEngine mLuaEngine;
    SDL_Window* mSdlWindow;
    SDL_GLContext mGlContext;
    bool mStopRendering = false;
    uint64_t mPrevTime;
    Renderer mRenderer;

    void startup()
    {
        mLuaEngine.startup();
        mLuaEngine.openlibs();

        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            SDL_Log("Otsukimi: Couldn't initialize SDL: %s", SDL_GetError());
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        mSdlWindow = SDL_CreateWindow("gegege::Otsukimi", 640, 480, SDL_WINDOW_OPENGL);
        if (!mSdlWindow)
        {
            SDL_Log("Otsukimi: Couldn't create window: %s", SDL_GetError());
            abort();
        }

        mGlContext = SDL_GL_CreateContext(mSdlWindow);
        if (!mGlContext)
        {
            SDL_Log("Otsukimi: Couldn't create gl context: %s", SDL_GetError());
        }

        gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
        SDL_Log("OpenGL Version: %s", (const char*)glGetString(GL_VERSION));
        SDL_Log("OpenGL Renderer: %s", (const char*)glGetString(GL_RENDERER));
        SDL_Log("GLSL Version: %s", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

        // set VSYNC
        if (!SDL_GL_SetSwapInterval(-1))
        {
            SDL_Log("VSYNC: %s", SDL_GetError());
            if (!SDL_GL_SetSwapInterval(1)) {
                SDL_Log("VSYNC: %s", SDL_GetError());
            }
        }

        mRenderer.startup();
        gRenderer = &mRenderer;

        lua_register(mLuaEngine.mL, "textureFind", textureFind);
        lua_register(mLuaEngine.mL, "drawTexture", drawTexture);
        lua_register(mLuaEngine.mL, "fontFind", fontFind);
        lua_register(mLuaEngine.mL, "drawText", drawText);

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

        mPrevTime = SDL_GetPerformanceCounter();
        SDL_Delay(1);
    }

    void shutdown()
    {
        mRenderer.shutdown();

        SDL_Quit();

        mLuaEngine.shutdown();
    }

    void run()
    {
        SDL_Event e;
        bool bQuit = false;

        while (!bQuit)
        {
            while (SDL_PollEvent(&e) != 0)
            {
                if (e.type == SDL_EVENT_QUIT)
                {
                    SDL_Log("Otsukimi: SDL_EVENT_QUIT is occured");
                    bQuit = true;
                }

                if (e.type == SDL_EVENT_WINDOW_MINIMIZED)
                {
                    SDL_Log("Otsukimi: SDL_EVENT_WINDOW_MINIMIZED is occured");
                    mStopRendering = true;
                }
                if (e.type == SDL_EVENT_WINDOW_RESTORED)
                {
                    SDL_Log("Otsukimi: SDL_EVENT_WINDOW_RESTORED is occured");
                    mStopRendering = false;
                    mPrevTime = SDL_GetPerformanceCounter();
                    SDL_Delay(1);
                }
            }

            if (mStopRendering)
            {
                SDL_Delay(100);
                continue;
            }

            int w, h;
            SDL_GetWindowSizeInPixels(mSdlWindow, &w, &h);
            mRenderer.update(0, 0, w, h);

            uint64_t now = SDL_GetPerformanceCounter();
            double dt = double(now - mPrevTime) / SDL_GetPerformanceFrequency();
            mPrevTime = now;

            mLuaEngine.call("update", gegege::lua::LuaNumber::make(dt));

            mRenderer.flush();

            SDL_GL_SwapWindow(mSdlWindow);
        }
    }
};

} // namespace gegege::otsukimi
