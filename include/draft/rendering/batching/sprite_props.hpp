#pragma once

#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "draft/rendering/material.hpp"

namespace Draft {
    /// Properties to render a sprite
    struct SpriteProps {
        // Variables
        Vector2f position{0, 0};
        float rotation = 0.f;
        Vector2f size{1, 1};
        Vector2f origin{0, 0};
        float zIndex = 0.f;

        FloatRect textureRegion{};
        Material2D material;
        
        bool operator()(SpriteProps const& a, SpriteProps const& b){ return a.zIndex > b.zIndex; }
    };
}