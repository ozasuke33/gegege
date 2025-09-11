#include "../../../include/gegege/otsukimi/otsukimi.hpp"

#define GLAD_GL_IMPLEMENTATION
#include "../../../include/gegege/otsukimi/gl.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace gegege::otsukimi {

Renderer* gRenderer;

void Otsukimi::startup()
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

    mRenderer.mTargetOffscreenWidth = 1280;
    mRenderer.mTargetOffscreenHeight = 720;
    mRenderer.startup();
    gRenderer = &mRenderer;

    mPrevTime = SDL_GetPerformanceCounter();
    SDL_Delay(1);
}

void Otsukimi::shutdown()
{
    mRenderer.shutdown();

    SDL_Quit();
}

void Otsukimi::run()
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

                onResized(e.window.data1, e.window.data2);
            }

            if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
            {
                std::vector<bool> button;
                button.emplace_back(e.button.button == SDL_BUTTON_LEFT);
                button.emplace_back(e.button.button == SDL_BUTTON_MIDDLE);
                button.emplace_back(e.button.button == SDL_BUTTON_RIGHT);
                button.emplace_back(e.button.button == SDL_BUTTON_X1);
                button.emplace_back(e.button.button == SDL_BUTTON_X2);
                onMousePressed(e.button.x, e.button.y, button);
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

                onKeyPressed(keyName, scancodeName, isRepeat);
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

        onUpdate(dt);

        mRenderer.flush();

        mRenderer.postUpdate(0, 0, w, h);

        mRenderer.flush();

        SDL_GL_SwapWindow(mSdlWindow);
    }
}
} // namespace gegege::otsukimi
