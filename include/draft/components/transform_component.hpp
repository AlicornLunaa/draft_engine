#pragma once

#include "draft/math/transform.hpp"
#include "draft/math/vector2.hpp"

namespace Draft {
    struct TransformComponent {
        // Variables
        Vector2f position;
        float rotation;

        // Constructors
        TransformComponent(const TransformComponent& transform) = default;
        TransformComponent(Vector2f position, float rotation, Vector2f scale) : position(position), rotation(rotation) {}
        TransformComponent(Vector2f position, float rotation) : position(position), rotation(rotation) {}
        TransformComponent() {}

        // Operators
        operator Transform () {
            auto t = Transform();
            t.translate(position.x, position.y);
            t.rotate(rotation);
            return t;
        }
    };
}