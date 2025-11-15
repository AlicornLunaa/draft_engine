#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/batching/shape_point.hpp"
#include "draft/rendering/batching/sprite_props.hpp"

namespace Draft {
    /// Simple wrapper for containing the matrices and the spriteproperties
    struct SpriteDrawCommand {
        SpriteProps sprite;
        Matrix4 projectionMatrix{1.f};
        Matrix4 transformMatrix{1.f};
        bool matricesDirty = false; // If this is true, the batch should reupload them
        
        bool operator()(SpriteDrawCommand const& a, SpriteDrawCommand const& b){ return a.sprite.zIndex > b.sprite.zIndex; }
    };

    struct ShapeDrawCommand {
        std::vector<ShapePoint> points = {};
        Matrix4 projectionMatrix{1.f};
        Matrix4 transformMatrix{1.f};
        ShapeRenderType type = ShapeRenderType::LINE;
        Shader const* shader = nullptr;
        float zLayer = 0.f;
    };
}