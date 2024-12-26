#pragma once

#include "draft/rendering/font.hpp"
#include "draft/rendering/texture.hpp"

struct RenderCommand {
    // Global data for rendering
    enum Command { DRAW_SPRITE, DRAW_TEXT, BEGIN_SCISSOR, END_SCISSOR };
    Command type;

    // Structures
    struct SpriteData {
        const Draft::Texture* texture;
        float regionX, regionY, regionWidth, regionHeight;
        float positionX, positionY;
        float sizeX, sizeY;
        float colorR, colorG, colorB, colorA;
    };

    // Data to draw text to the screen
    struct TextData {
        const char* string;
        uint stringLength;
        Draft::Font* font;
        int fontSize;
        float positionX, positionY;
        float originX, originY;
        float colorR, colorG, colorB, colorA;
    };

    // Data to start a scissor box
    struct ScissorData {
        float x;
        float y;
        float width;
        float height;
    };

    // Unionizing
    union {
        SpriteData sprite;
        TextData text;
        ScissorData scissor;
    };
};