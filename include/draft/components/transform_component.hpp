#pragma once

#include "draft/math/vector2.hpp"
#include "draft/math/matrix.hpp"

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
        operator Matrix4 () {
            auto t = Matrix4::identity();
            t *= Matrix4::translation({ position.x, position.y, 0 });
            t *= Matrix4::rotation({ 0, 0, rotation });
            return t;
        }
    };
}