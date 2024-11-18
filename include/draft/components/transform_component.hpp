#pragma once

#include "draft/math/glm.hpp"

namespace Draft {
    /**
     * @brief Position and rotation, thats it
     */
    struct TransformComponent {
    private:
        // Private vars
        Vector2f dp = {};
        float dr = 0.f;

    public:
        // Variables
        Vector2f position = {};
        float rotation = 0.f;

        // Constructors
        TransformComponent(const TransformComponent& transform) = default;
        TransformComponent(Vector2f position, float rotation) : position(position), rotation(rotation) {}
        TransformComponent() {}

        // Friends
        friend class PhysicsSystem;

        // Operators
        operator Matrix4 () {
            auto t = Matrix4(1.f);
            t = Math::translate(t, { position.x, position.y, 0 });
            t = Math::rotate(t, rotation, { 0, 0, 1 });
            return t;
        }
    };
}