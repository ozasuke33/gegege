#pragma once

#include "renderer.hpp"

namespace gegege::otsukimi {

extern Renderer* gRenderer;

Texture* textureFind(const std::string& path)
{
    return gRenderer->textureFind(path);
}

int getTextureWidth(Texture* tex)
{
    return tex->mWidth;
}

int getTextureHeight(Texture* tex)
{
    return tex->mHeight;
}

void drawTexture(Texture* tex, float sx, float sy, float sw, float sh, float scaleX, float scaleY, float angle, float dx, float dy, float r, float g, float b, float a)
{
    gRenderer->drawTexture(tex, sx, sy, sw, sh, scaleX, scaleY, angle, dx, dy, r, g, b, a);
}

TTF_Font* fontFind(const std::string& path, float ptSize)
{
    return gRenderer->fontFind(path, ptSize);
}

void drawText(TTF_Font* font, float x, float y, const std::string& text, float r, float g, float b, float a)
{
    gRenderer->drawText(font, x, y, text, r, g, b, a);
}

void setFontOutline(TTF_Font* font, int outline)
{
    TTF_SetFontOutline(font, outline);
}

} // namespace gegege::otsukimi
