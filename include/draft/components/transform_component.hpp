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

        // Functions
        Matrix3 get_transform() const {
            Matrix3 mat = Matrix3(1.f);
            mat = Math::translate(mat, position);
            mat = Math::rotate(mat, rotation);
            return mat;
        }

        Matrix4 get_matrix() const {
            Matrix4 mat = Matrix4(1.f);
            mat = Math::translate(mat, {position, 0.f});
            mat = Math::rotate(mat, rotation, {0, 0, 1});
            return mat;
        }

        // Operators
        operator Matrix4 () {
            return get_matrix();
        }
    };

    typedef TransformComponent Transform;
}