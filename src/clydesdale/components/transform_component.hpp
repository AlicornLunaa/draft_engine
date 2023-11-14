#pragma once
#include <SFML/Graphics.hpp>

namespace Clydesdale::ECS {
    struct TransformComponent {
        sf::Transform transform = sf::Transform::Identity;

        TransformComponent(const TransformComponent& transform) = default;

        TransformComponent(sf::Vector2f position, float rotation, sf::Vector2f scale){
            transform.scale(scale);
            transform.rotate(rotation);
            transform.translate(position);
        }

        TransformComponent(sf::Vector2f position, float rotation){
            transform.rotate(rotation);
            transform.translate(position);
        }

        TransformComponent() {}

        operator sf::Transform& (){ return transform; }
        operator const sf::Transform& (){ return transform; }
    };
}