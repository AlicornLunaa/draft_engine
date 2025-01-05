#pragma once

#include "draft/rendering/font.hpp"
#include "draft/util/color.hpp"

namespace Draft {
    namespace UI {
        struct Command {
            enum Type { SPRITE, TEXT, SHAPE, START_SCISSOR, END_SCISSOR } type;

            float x;
            float y;
            Color color;

            union {
                struct {
                    const Texture* texture;
                    float width;
                    float height;
                } sprite;

                struct {
                    const char* str;
                    Font* font;
                    uint fontSize;
                } text;

                struct {
                    Vector2f pos;
                    Vector2f size;
                } shape;

                struct {
                    float width;
                    float height;
                } scissor;
            };
        };
    };
};