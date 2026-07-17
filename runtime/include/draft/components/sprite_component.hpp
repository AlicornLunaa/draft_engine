#pragma once

#include "draft/asset/resource.hpp"
#include "draft/math/glm.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/reflectable.hpp"

#include <optional>

namespace Draft {
    /**
     * @brief A texture region plus a world-space size/origin/z-index. Rendered by RenderSystem
     * together with a TransformComponent on the same entity.
     */
    struct SpriteComponent {
        DRAFT_REFLECTED(TextureRegion, texture) = {};
        DRAFT_REFLECTED(Vector2f, size) = { 64, 64 };
        DRAFT_REFLECTED(Vector2f, origin) = { 0, 0 };
        DRAFT_REFLECTED(float, zIndex) = 0.f;

        // None means use SpriteCollection's shared default shader
        std::optional<Resource<Shader>> shader;

        SpriteComponent() = default;
        SpriteComponent(Resource<Texture> texture, const Vector2f& size, const Vector2f& origin = {}) : texture(TextureRegion{texture, {}}), size(size), origin(origin) {}
        SpriteComponent(TextureRegion texture, const Vector2f& size, const Vector2f& origin = {}) : texture(texture), size(size), origin(origin) {}

        DRAFT_REFLECTABLE(SpriteComponent, texture, size, origin, zIndex)
    };
}
