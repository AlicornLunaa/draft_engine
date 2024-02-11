#pragma once
#include <clydesdale/math.hpp>

namespace Clydesdale::ECS {
    struct TransformComponent {
        // Variables
        Math::Transform transform;

        // Constructors
        TransformComponent(const TransformComponent& transform) = default;

        TransformComponent(Math::Vector2f position, float rotation, Math::Vector2f scale){
            transform.scale(scale);
            transform.rotate(rotation);
            transform.translate(position);
        }

        TransformComponent(Math::Vector2f position, float rotation){
            transform.scale(1, 1);
            transform.rotate(rotation);
            transform.translate(position);
        }

        TransformComponent() {}

        // Operators
        operator Math::Transform& (){ return transform; }
        explicit operator const Math::Transform& (){ return transform; }
    };
}