#pragma once

#include "draft/math/glm.hpp"
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
        Vector2f origin = { 0, 0 };
        float zIndex = 0.f;

        // Constructors
        SpriteComponent(const SpriteComponent& transform) = default;
        SpriteComponent(const Texture& texture, const Vector2f& size, const Vector2f& origin = {}) : texture(texture), size(size), origin(origin) {}
    };
}