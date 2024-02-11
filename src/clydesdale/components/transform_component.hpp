#pragma once
#include "clydesdale/math/transform.hpp"
#include "clydesdale/math/vector2.hpp"

namespace Clydesdale {
    struct TransformComponent {
        // Variables
        Transform transform;

        // Constructors
        TransformComponent(const TransformComponent& transform) = default;

        TransformComponent(Vector2f position, float rotation, Vector2f scale){
            transform.scale(scale);
            transform.rotate(rotation);
            transform.translate(position);
        }

        TransformComponent(Vector2f position, float rotation){
            transform.scale(1, 1);
            transform.rotate(rotation);
            transform.translate(position);
        }

        TransformComponent() {}

        // Operators
        operator Transform& (){ return transform; }
        explicit operator const Transform& (){ return transform; }
    };
}