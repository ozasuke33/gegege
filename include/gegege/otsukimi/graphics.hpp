#pragma once

#include "renderer.hpp"

namespace gegege::otsukimi {

extern Renderer* gRenderer;

Texture* textureFind(const std::string& path);

int getTextureWidth(Texture* tex);

int getTextureHeight(Texture* tex);

void drawTexture(Texture* tex, float sx, float sy, float sw, float sh, float scaleX, float scaleY, float angle, float dx, float dy, float r, float g, float b, float a);

TTF_Font* fontFind(const std::string& path, float ptSize);

void drawText(TTF_Font* font, float x, float y, const std::string& text, float r, float g, float b, float a);

void setFontOutline(TTF_Font* font, int outline);

} // namespace gegege::otsukimi
