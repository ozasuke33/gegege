#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include <gegege/otsukimi/gl.h>

#include <string>
#include <unordered_map>
#include <filesystem>
#include <vector>

#include <stb_image.h>

namespace gegege::otsukimi {

constexpr unsigned int FRAME_OVERLAP = 2;
constexpr unsigned int MAX_VERTEX = 1024;

struct Vertex {
    float mX;
    float mY;
    float mU;
    float mV;
    float mR;
    float mG;
    float mB;
    float mA;
};

struct Texture {
    GLuint mTexID;
    GLuint mFramebufferID;
    int mWidth;
    int mHeight;
};

struct VBO {
    GLuint mVertexBufferID;
    GLsizeiptr mNumBytes;
};

struct FrameData {
    VBO* mVertexBuffer;
    std::vector<Texture*> mTextTextures;
    std::vector<Texture*> mTextures;
    std::vector<Vertex> mVertices;
    Texture* mFrameBuffer;
};

struct Renderer {
    GLuint mVAO;
    GLuint mShader;
    GLint mMVPLocation;

    FrameData mFrames[FRAME_OVERLAP];
    uint32_t mFrameNumber;
    FrameData& getCurrentFrame() { return mFrames[mFrameNumber % FRAME_OVERLAP]; }

    std::unordered_map<std::string, Texture*> mTextures;
    std::unordered_map<std::string, TTF_Font*> mFonts;

    int mTargetWidth;
    int mTargetHeight;

    void startup()
    {
        if (!TTF_Init())
        {
            SDL_Log("Couldn't initialise SDL_ttf: %s", SDL_GetError());
        }

        glGenVertexArrays(1, &mVAO);
        glBindVertexArray(mVAO);

        mShader = createShader(
            R"(#version 330
layout(location = 0)in vec2 vPosition;
layout(location = 1)in vec2 vTexCoord;
layout(location = 2)in vec4 vColor;
uniform mat4 uMVP;
out vec2 pTexCoord;
out vec4 pColor;
void main()
{
    gl_Position = uMVP * vec4(vPosition.x, vPosition.y, 0.0, 1.0);
    pTexCoord = vTexCoord;
    pColor = vColor;
}
)",
            R"(#version 330
in vec2 pTexCoord;
in vec4 pColor;
uniform sampler2D uTex;
out vec4 fragColor;
void main()
{
    fragColor = texture(uTex, pTexCoord) * pColor;
})");
        glUseProgram(mShader);

        mMVPLocation = glGetUniformLocation(mShader, "uMVP");

        for (auto& i : mFrames)
        {
            i.mVertexBuffer = createVBO(sizeof(Vertex) * MAX_VERTEX);
            i.mFrameBuffer = createFBO(mTargetWidth, mTargetHeight);
        }

        glActiveTexture(GL_TEXTURE0);
    }

    void shutdown()
    {
        TTF_Quit();
    }

    void update(GLint viewportX, GLint viewportY, GLsizei viewportWidth, GLsizei viewportHeight)
    {
        mFrameNumber++;

        FrameData& frame = getCurrentFrame();
        for (Texture* tex : frame.mTextTextures)
        {
            glDeleteTextures(1, &tex->mTexID);
            delete tex;
        }
        frame.mTextTextures.clear();
        frame.mTextures.clear();
        frame.mVertices.clear();

        glBindFramebuffer(GL_FRAMEBUFFER, frame.mFrameBuffer->mFramebufferID);

        glViewport(0, 0, mTargetWidth, mTargetHeight);
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

        glm::mat4 ortho = glm::ortho(float(-mTargetWidth) / 2.0f, float(mTargetWidth) / 2.0f, float(-mTargetHeight) / 2.0f, float(mTargetHeight) / 2.0f);

        glUniformMatrix4fv(mMVPLocation, 1, GL_FALSE, glm::value_ptr(ortho));
    }

    void postUpdate(GLint viewportX, GLint viewportY, GLsizei viewportWidth, GLsizei viewportHeight)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

        glm::mat4 ortho = glm::ortho(float(-viewportWidth) / 2.0f, float(viewportWidth) / 2.0f, float(-viewportHeight) / 2.0f, float(viewportHeight) / 2.0f);

        glUniformMatrix4fv(mMVPLocation, 1, GL_FALSE, glm::value_ptr(ortho));

        float scale = std::min(float(viewportWidth) / mTargetWidth, float(viewportHeight) / mTargetHeight);
        FrameData& frame = getCurrentFrame();
        drawTexture(frame.mFrameBuffer, 0, 0, frame.mFrameBuffer->mWidth, frame.mFrameBuffer->mHeight, scale, -scale, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    }

    GLuint createShader(const char* vert, const char* frag)
    {
        GLint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vert, NULL);
        glCompileShader(vs);
        GLint vsCompileStatus;
        glGetShaderiv(vs, GL_COMPILE_STATUS, &vsCompileStatus);
        if (vsCompileStatus == GL_FALSE)
        {
            GLint logLength;
            glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &logLength);
            if (logLength)
            {
                std::string infoLog(logLength, ' ');
                glGetShaderInfoLog(vs, logLength, NULL, &infoLog[0]);
                SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Vertex shader compilation failed: %s", infoLog.c_str());
            }
        }
        SDL_assert_release(vsCompileStatus == GL_TRUE);

        GLint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &frag, NULL);
        glCompileShader(fs);
        GLint fsCompileStatus;
        glGetShaderiv(fs, GL_COMPILE_STATUS, &fsCompileStatus);
        if (fsCompileStatus == GL_FALSE)
        {
            GLint logLength;
            glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &logLength);
            if (logLength)
            {
                std::string infoLog(logLength, ' ');
                glGetShaderInfoLog(fs, logLength, NULL, &infoLog[0]);
                SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Fragment shader compilation failed: %s", infoLog.c_str());
            }
        }
        SDL_assert_release(fsCompileStatus == GL_TRUE);

        GLuint program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);

        GLint programLinkStatus;
        glGetProgramiv(program, GL_LINK_STATUS, &programLinkStatus);
        if (programLinkStatus == GL_FALSE)
        {
            GLint logLength;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
            if (logLength)
            {
                std::string infoLog(logLength, ' ');
                glGetProgramInfoLog(program, logLength, NULL, &infoLog[0]);
                SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Shader link failed: %s", infoLog.c_str());
            }
        }
        SDL_assert_release(programLinkStatus == GL_TRUE);

        glDetachShader(program, vs);
        glDetachShader(program, fs);
        glDeleteShader(vs);
        glDeleteShader(fs);

        return program;
    }

    Texture* createFBO(int width, int height)
    {
        Texture* fbo = new Texture();
        glGenFramebuffers(1, &fbo->mFramebufferID);
        SDL_assert_release(fbo->mFramebufferID);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo->mFramebufferID);

        glGenTextures(1, &fbo->mTexID);
        SDL_assert_release(fbo->mTexID);
        glBindTexture(GL_TEXTURE_2D, fbo->mTexID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo->mTexID, 0);

        fbo->mWidth = width;
        fbo->mHeight = height;

        GLenum attachments[2] = {GL_COLOR_ATTACHMENT0, GL_NONE};
        glDrawBuffers(1, attachments);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        SDL_assert_release(status == GL_FRAMEBUFFER_COMPLETE);

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return fbo;
    }

    VBO* createVBO(GLsizeiptr numBytes)
    {
        VBO* vbo = new VBO();

        glGenBuffers(1, &vbo->mVertexBufferID);
        SDL_assert_release(vbo->mVertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, vbo->mVertexBufferID);
        glBufferData(GL_ARRAY_BUFFER, numBytes, NULL, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(2 * sizeof(float)));
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        vbo->mNumBytes = numBytes;

        return vbo;
    }

    Texture* textureFind(const std::string& path)
    {
        if (mTextures.contains(path))
        {
            return mTextures[path];
        }

        std::filesystem::path basePath = SDL_GetBasePath();
        basePath.append("data");
        basePath.append(path);

        Texture* tex = new Texture();
        int c;
        unsigned char* data = stbi_load(basePath.generic_string().c_str(), &tex->mWidth, &tex->mHeight, &c, 0);
        if (!data)
        {
            SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Texture failed to load: %s", basePath.generic_string().c_str());
            return nullptr;
        }
        else
        {
            SDL_Log("Texture loaded: %s %dx%d", basePath.generic_string().c_str(), tex->mWidth, tex->mHeight);
        }

        glGenTextures(1, &tex->mTexID);
        SDL_assert_release(tex);
        glBindTexture(GL_TEXTURE_2D, tex->mTexID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if (c == 3)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, tex->mWidth, tex->mHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else if (c == 4)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex->mWidth, tex->mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        stbi_image_free(data);

        glBindTexture(GL_TEXTURE_2D, 0);

        mTextures[path] = tex;

        return mTextures[path];
    }

    void drawTexture(Texture* tex, float sx, float sy, float sw, float sh, float scaleX, float scaleY, float angle, float dx, float dy, float r, float g, float b, float a)
    {
        // (-1,  1)  - ( 1,  1)
        //     |           |
        // (-1, -1)  - ( 1, -1)

        float sourceX = sx / tex->mWidth;
        float sourceY = sy / tex->mHeight;
        float sourceW = (sx + sw) / tex->mWidth;
        float sourceH = (sy + sh) / tex->mHeight;

        Vertex topLeft;
        topLeft.mX = -(sw / 2.0f);
        topLeft.mY = (sh / 2.0f);
        topLeft.mU = sourceX;
        topLeft.mV = sourceY;
        topLeft.mR = r;
        topLeft.mG = g;
        topLeft.mB = b;
        topLeft.mA = a;

        Vertex topRight;
        topRight.mX = (sw / 2.0f);
        topRight.mY = (sh / 2.0f);
        topRight.mU = sourceW;
        topRight.mV = sourceY;
        topRight.mR = r;
        topRight.mG = g;
        topRight.mB = b;
        topRight.mA = a;

        Vertex bottomLeft;
        bottomLeft.mX = -(sw / 2.0f);
        bottomLeft.mY = -(sh / 2.0f);
        bottomLeft.mU = sourceX;
        bottomLeft.mV = sourceH;
        bottomLeft.mR = r;
        bottomLeft.mG = g;
        bottomLeft.mB = b;
        bottomLeft.mA = a;

        Vertex bottomRight;
        bottomRight.mX = (sw / 2.0f);
        bottomRight.mY = -(sh / 2.0f);
        bottomRight.mU = sourceW;
        bottomRight.mV = sourceH;
        bottomRight.mR = r;
        bottomRight.mG = g;
        bottomRight.mB = b;
        bottomRight.mA = a;

        FrameData& frame = getCurrentFrame();

        if (frame.mVertices.size() >= MAX_VERTEX)
        {
            flush();
        }

        frame.mTextures.emplace_back(tex);

        glm::mat4 Translate = glm::translate(glm::mat4(1.0f), glm::vec3(dx, dy, 0.0f));
        glm::mat4 Rotate = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 Scale = glm::scale(glm::mat4(1.0f), glm::vec3(scaleX, scaleY, 1.0f));
        glm::mat4 TRS = Translate * Rotate * Scale;

        glm::vec4 v;

        v = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        v.x = topLeft.mX;
        v.y = topLeft.mY;
        v = TRS * v;
        topLeft.mX = v.x;
        topLeft.mY = v.y;

        v = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        v.x = topRight.mX;
        v.y = topRight.mY;
        v = TRS * v;
        topRight.mX = v.x;
        topRight.mY = v.y;

        v = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        v.x = bottomLeft.mX;
        v.y = bottomLeft.mY;
        v = TRS * v;
        bottomLeft.mX = v.x;
        bottomLeft.mY = v.y;

        v = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        v.x = bottomRight.mX;
        v.y = bottomRight.mY;
        v = TRS * v;
        bottomRight.mX = v.x;
        bottomRight.mY = v.y;

        frame.mVertices.emplace_back(topLeft);
        frame.mVertices.emplace_back(bottomLeft);
        frame.mVertices.emplace_back(bottomRight);

        frame.mVertices.emplace_back(bottomRight);
        frame.mVertices.emplace_back(topRight);
        frame.mVertices.emplace_back(topLeft);
    }

    void flush()
    {
        FrameData& frame = getCurrentFrame();

        if (frame.mVertices.empty())
        {
            return;
        }

        glBindBuffer(GL_ARRAY_BUFFER, frame.mVertexBuffer->mVertexBufferID);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * frame.mVertices.size(), &frame.mVertices[0]);

        unsigned int j = 0;
        for (unsigned int i = 0; i < frame.mVertices.size(); i += 6)
        {
            Texture* tex = frame.mTextures[j];
            glBindTexture(GL_TEXTURE_2D, tex->mTexID);
            ++j;
            glDrawArrays(GL_TRIANGLES, i, 6);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        frame.mVertices.clear();
        frame.mTextures.clear();
    }

    TTF_Font* fontFind(const std::string& path, float ptSize)
    {
        if (mFonts.contains(path))
        {
            return mFonts[path];
        }

        std::filesystem::path basePath = SDL_GetBasePath();
        basePath.append("data");
        basePath.append(path);

        TTF_Font* font = TTF_OpenFont(basePath.generic_string().c_str(), ptSize);
        if (!font)
        {
            SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Font failed to load: %s", SDL_GetError());
        }
        else
        {
            SDL_Log("Font loaded: %s", basePath.generic_string().c_str());
        }

        mFonts[path] = font;

        return mFonts[path];
    }

    void drawText(TTF_Font* font, float x, float y, const std::string& text, float r, float g, float b, float a)
    {
        SDL_Color color = {Uint8(r * 255), Uint8(g * 255), Uint8(b * 255), Uint8(a * 255)};
        SDL_Surface* surf = TTF_RenderText_Blended_Wrapped(font, text.c_str(), 0, color, 0);
        if (!surf)
        {
            SDL_Log("%s", SDL_GetError());
        }

        SDL_Surface* rgbaSurf = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_ABGR8888);
        if (!rgbaSurf)
        {
            SDL_Log("%s", SDL_GetError());
        }
        SDL_DestroySurface(surf);

        Texture* tex = new Texture();
        tex->mWidth = rgbaSurf->w;
        tex->mHeight = rgbaSurf->h;

        glGenTextures(1, &tex->mTexID);
        SDL_assert_release(tex);
        glBindTexture(GL_TEXTURE_2D, tex->mTexID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex->mWidth, tex->mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaSurf->pixels);

        glBindTexture(GL_TEXTURE_2D, 0);

        SDL_DestroySurface(rgbaSurf);

        drawTexture(tex, 0, 0, tex->mWidth, tex->mHeight, 1, 1, 0, x, y, 1, 1, 1, 1);

        FrameData& frame = getCurrentFrame();
        frame.mTextTextures.emplace_back(tex);
    }
};

} // namespace gegege::otsukimi
