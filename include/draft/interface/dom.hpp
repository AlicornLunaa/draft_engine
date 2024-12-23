#pragma once

#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "draft/rendering/batching/text_renderer.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/rendering/font.hpp"
#include "draft/util/color.hpp"

#include <vector>

struct Element {
    Draft::Vector2f position;
    Draft::Vector2f size;
    
    Draft::Vector4f dimensions;
    Draft::Vector4f margin;
    Draft::Vector4f padding;

    Color color;
    Color backgroundColor;
    Draft::Texture const* texture = nullptr;

    float fontSize;
    Draft::Font* font = nullptr;

    bool visibility;
    Draft::FloatRect bounds;
    Element* parent = nullptr;
    std::vector<Element> children;

    std::vector<Draft::SpriteProps> sprites;
    std::vector<Draft::TextProperties> text;
};
