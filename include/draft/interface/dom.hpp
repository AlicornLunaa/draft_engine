#pragma once

#include "draft/interface/render_command.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
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

    std::vector<RenderCommand> renderCommands;
};
