#pragma once
#include <clydesdale/math.hpp>

namespace Clydesdale::ECS {
    struct TransformComponent {
        Math::Transform transform = Math::Transform::Identity;

        TransformComponent(const TransformComponent& transform) = default;

        TransformComponent(Math::Vector2f position, float rotation, Math::Vector2f scale){
            transform.scale(scale);
            transform.rotate(rotation);
            transform.translate(position);
        }

        TransformComponent(Math::Vector2f position, float rotation){
            transform.rotate(rotation);
            transform.translate(position);
        }

        TransformComponent() {}

        operator Math::Transform& (){ return transform; }
    };
}