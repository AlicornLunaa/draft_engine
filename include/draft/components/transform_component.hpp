#pragma once

#include "draft/math/glm.hpp"

namespace Draft {
    struct TransformComponent {
        // Variables
        Vector2f position = {};
        float rotation = 0.f;

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