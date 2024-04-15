#pragma once

#include "draft/math/vector2.hpp"
#include "draft/rendering/texture.hpp"

namespace Draft {
    /**
     * @brief Contains a texture and a size. Will be rendered with the transform component
     * 
     */
    struct SpriteComponent {
        // Variables
        const Texture& texture;
        Vector2f size = { 64, 64 };

        // Constructors
        SpriteComponent(const SpriteComponent& transform) = default;
        SpriteComponent(const Texture& texture, const Vector2f& size) : texture(texture), size(size) {}
    };
}