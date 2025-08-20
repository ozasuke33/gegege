#pragma once

#include <gegege/lua_engine/lua_engine.hpp>
#include <gegege/otsukimi/gl.h>
#include <gegege/otsukimi/renderer.hpp>

#include <filesystem>

namespace gegege::otsukimi {

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

            //Texture* tex = mRenderer.textureFind("bar/pic.png");
            //mRenderer.drawTexture(tex, 0, 0);
            //mRenderer.drawTexture(tex, 100, 100);
            //mRenderer.drawTexture(tex, -100, 100);

            mRenderer.flush();

            SDL_GL_SwapWindow(mSdlWindow);
        }
    }
};

} // namespace gegege::otsukimi
