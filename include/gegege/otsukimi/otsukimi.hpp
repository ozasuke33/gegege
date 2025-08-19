#pragma once

#include <gegege/lua_engine/lua_engine.hpp>

#include <filesystem>

namespace gegege::otsukimi {

struct Otsukimi {
    lua::LuaEngine mLuaEngine;
    SDL_Window* mSdlWindow;
    bool mStopRendering;

    void startup()
    {
        mLuaEngine.startup();
        mLuaEngine.openlibs();

        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            SDL_Log("Otsukimi: Couldn't initialize SDL: %s", SDL_GetError());
        }

        mSdlWindow = SDL_CreateWindow("gegege::Otsukimi", 640, 480, SDL_WINDOW_OPENGL);
        if (!mSdlWindow)
        {
            SDL_Log("Otsukimi: Couldn't create window: %s", SDL_GetError());
            abort();
        }

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

    void shutdown()
    {
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
                }
            }

            if (mStopRendering)
            {
                SDL_Delay(100);
                continue;
            }
        }
    }
};

} // namespace gegege::otsukimi
