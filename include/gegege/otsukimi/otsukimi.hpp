#pragma once

#include <gegege/lua_engine/lua_engine.hpp>
#include <gegege/otsukimi/gl.h>
#include <gegege/otsukimi/renderer.hpp>

#include <filesystem>

namespace gegege::otsukimi {

Renderer* gRenderer;

int getMouseCoordinateToScreenCoordinateX(lua_State* L)
{
    lua::LuaEngine lua;
    lua.mL = L;
    lua::LuaValue mouseX = lua.popValue();

    float NdcX = 2.0f * std::get<lua::LuaNumber>(mouseX).mValue / gRenderer->mScreenWidth - 1.0f;
    glm::mat4 ortho = glm::ortho(float(-gRenderer->mScreenWidth) / 2.0f, float(gRenderer->mScreenWidth) / 2.0f, float(-gRenderer->mScreenHeight) / 2.0f, float(gRenderer->mScreenHeight) / 2.0f);
    float scale = std::min(float(gRenderer->mScreenWidth) / gRenderer->mTargetWidth, float(gRenderer->mScreenHeight) / gRenderer->mTargetHeight);
    glm::vec4 orthoCoords = glm::inverse(ortho) * glm::vec4(NdcX / scale, 0.0f, 0.0f, 1.0f);

    lua.pushValue(lua::LuaNumber::make(orthoCoords.x));

    return 1;
}

int getMouseCoordinateToScreenCoordinateY(lua_State* L)
{
    lua::LuaEngine lua;
    lua.mL = L;
    lua::LuaValue mouseY = lua.popValue();

    float NdcY = 1.0f - 2.0f * std::get<lua::LuaNumber>(mouseY).mValue / gRenderer->mScreenHeight;
    glm::mat4 ortho = glm::ortho(float(-gRenderer->mScreenWidth) / 2.0f, float(gRenderer->mScreenWidth) / 2.0f, float(-gRenderer->mScreenHeight) / 2.0f, float(gRenderer->mScreenHeight) / 2.0f);
    float scale = std::min(float(gRenderer->mScreenWidth) / gRenderer->mTargetWidth, float(gRenderer->mScreenHeight) / gRenderer->mTargetHeight);
    glm::vec4 orthoCoords = glm::inverse(ortho) * glm::vec4(0.0f, NdcY / scale, 0.0f, 1.0f);

    lua.pushValue(lua::LuaNumber::make(orthoCoords.y));

    return 1;
}

int textureFind(lua_State* L)
{
    lua::LuaEngine lua;
    lua.mL = L;
    lua::LuaValue path = lua.popValue();
    Texture* tex = gRenderer->textureFind(lua::getLuaValueString(path));
    lua_pushlightuserdata(L, tex);
    return 1;
}

int getTextureWidth(lua_State* L)
{
    lua::LuaEngine lua;
    lua.mL = L;
    Texture* tex = (Texture*)lua_touserdata(L, -1);
    lua.pushValue(lua::LuaNumber::make(tex->mWidth));
    return 1;
}

int getTextureHeight(lua_State* L)
{
    lua::LuaEngine lua;
    lua.mL = L;
    Texture* tex = (Texture*)lua_touserdata(L, -1);
    lua.pushValue(lua::LuaNumber::make(tex->mHeight));
    return 1;
}

int drawTexture(lua_State* L)
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

    gRenderer->drawTexture(tex,
                           std::get<lua::LuaNumber>(sx).mValue, std::get<lua::LuaNumber>(sy).mValue, std::get<lua::LuaNumber>(sw).mValue, std::get<lua::LuaNumber>(sh).mValue,
                           std::get<lua::LuaNumber>(scaleX).mValue, std::get<lua::LuaNumber>(scaleY).mValue,
                           std::get<lua::LuaNumber>(angle).mValue,
                           std::get<lua::LuaNumber>(dx).mValue, std::get<lua::LuaNumber>(dy).mValue,
                           std::get<lua::LuaNumber>(r).mValue, std::get<lua::LuaNumber>(g).mValue, std::get<lua::LuaNumber>(b).mValue, std::get<lua::LuaNumber>(a).mValue);
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
    lua::LuaValue a = lua.popValue();
    lua::LuaValue b = lua.popValue();
    lua::LuaValue g = lua.popValue();
    lua::LuaValue r = lua.popValue();
    lua::LuaValue text = lua.popValue();
    lua::LuaValue y = lua.popValue();
    lua::LuaValue x = lua.popValue();
    TTF_Font* font = (TTF_Font*)lua_touserdata(L, -1);
    gRenderer->drawText(font, std::get<lua::LuaNumber>(x).mValue, std::get<lua::LuaNumber>(y).mValue,
                        lua::getLuaValueString(text),
                        std::get<lua::LuaNumber>(r).mValue, std::get<lua::LuaNumber>(g).mValue, std::get<lua::LuaNumber>(b).mValue, std::get<lua::LuaNumber>(a).mValue);
    return 0;
}

int setFontOutline(lua_State* L)
{
    lua::LuaEngine lua;
    lua.mL = L;
    lua::LuaValue outlineSize = lua.popValue();
    TTF_Font* font = (TTF_Font*)lua_touserdata(L, -1);
    TTF_SetFontOutline(font, std::get<lua::LuaNumber>(outlineSize).mValue);
    return 0;
}

struct Otsukimi {
    lua::LuaEngine mLuaEngine;
    SDL_Window* mSdlWindow;
    SDL_GLContext mGlContext;
    bool mStopRendering = false;
    uint64_t mPrevTime;
    Renderer mRenderer;
    bool mFullscreen;

    void startup()
    {
        mLuaEngine.startup();
        mLuaEngine.openlibs();

        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            SDL_Log("Otsukimi: Couldn't initialize SDL: %s", SDL_GetError());
        }

        mFullscreen = false;

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        mSdlWindow = SDL_CreateWindow("gegege::Otsukimi", 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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
            if (!SDL_GL_SetSwapInterval(1))
            {
                SDL_Log("VSYNC: %s", SDL_GetError());
            }
        }

        mRenderer.mTargetWidth = 1280;
        mRenderer.mTargetHeight = 720;
        mRenderer.startup();
        gRenderer = &mRenderer;

        lua_register(mLuaEngine.mL, "getMouseCoordinateToScreenCoordinateX", getMouseCoordinateToScreenCoordinateX);
        lua_register(mLuaEngine.mL, "getMouseCoordinateToScreenCoordinateY", getMouseCoordinateToScreenCoordinateY);
        lua_register(mLuaEngine.mL, "textureFind", textureFind);
        lua_register(mLuaEngine.mL, "getTextureWidth", getTextureWidth);
        lua_register(mLuaEngine.mL, "getTextureHeight", getTextureHeight);
        lua_register(mLuaEngine.mL, "drawTexture", drawTexture);
        lua_register(mLuaEngine.mL, "fontFind", fontFind);
        lua_register(mLuaEngine.mL, "drawText", drawText);
        lua_register(mLuaEngine.mL, "setFontOutline", setFontOutline);

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

                if (e.type == SDL_EVENT_WINDOW_RESIZED)
                {
                    gRenderer->mScreenWidth = e.window.data1;
                    gRenderer->mScreenHeight = e.window.data2;

                    int type = lua_getglobal(mLuaEngine.mL, "onResized");
                    if (type == LUA_TFUNCTION)
                    {
                        mLuaEngine.call("onResized", lua::LuaNumber::make(e.window.data1), lua::LuaNumber::make(e.window.data2));
                    }
                    lua_pop(mLuaEngine.mL, 1);
                }

                if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
                {
                    int type = lua_getglobal(mLuaEngine.mL, "onMousePressed");
                    if (type == LUA_TFUNCTION)
                    {
                        mLuaEngine.pushValue(lua::LuaNumber::make(e.button.x));
                        mLuaEngine.pushValue(lua::LuaNumber::make(e.button.y));
                        lua_newtable(mLuaEngine.mL);
                        lua_pushboolean(mLuaEngine.mL, e.button.button == SDL_BUTTON_LEFT);
                        lua_rawseti(mLuaEngine.mL, -2, 1);
                        lua_pushboolean(mLuaEngine.mL, e.button.button == SDL_BUTTON_MIDDLE);
                        lua_rawseti(mLuaEngine.mL, -2, 2);
                        lua_pushboolean(mLuaEngine.mL, e.button.button == SDL_BUTTON_RIGHT);
                        lua_rawseti(mLuaEngine.mL, -2, 3);
                        lua_pushboolean(mLuaEngine.mL, e.button.button == SDL_BUTTON_X1);
                        lua_rawseti(mLuaEngine.mL, -2, 4);
                        lua_pushboolean(mLuaEngine.mL, e.button.button == SDL_BUTTON_X2);
                        lua_rawseti(mLuaEngine.mL, -2, 5);
                        mLuaEngine.pcall(3, 1);
                    }
                    lua_pop(mLuaEngine.mL, 1);
                }

                if (e.type == SDL_EVENT_KEY_DOWN)
                {
                    if (e.key.mod & SDL_KMOD_ALT && e.key.key == SDLK_RETURN)
                    {
                        mFullscreen = !mFullscreen;
                        SDL_SetWindowFullscreen(mSdlWindow, mFullscreen);
                    }

                    std::string keyName(SDL_GetKeyName(e.key.key));
                    std::string scancodeName(SDL_GetScancodeName(e.key.scancode));
                    bool isRepeat = e.key.repeat;

                    int type = lua_getglobal(mLuaEngine.mL, "onKeyPressed");
                    mLuaEngine.popValue();
                    if (type == LUA_TFUNCTION)
                    {
                        mLuaEngine.call("onKeyPressed", lua::LuaString::make(keyName), lua::LuaString::make(scancodeName), lua::LuaBoolean::make(isRepeat));
                    }
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

            int type = lua_getglobal(mLuaEngine.mL, "onUpdate");
            mLuaEngine.popValue();
            if (type == LUA_TFUNCTION)
            {
                mLuaEngine.call("onUpdate", gegege::lua::LuaNumber::make(dt));
            }

            mRenderer.flush();

            mRenderer.postUpdate(0, 0, w, h);

            mRenderer.flush();

            SDL_GL_SwapWindow(mSdlWindow);
        }
    }
};

} // namespace gegege::otsukimi
