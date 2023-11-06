#pragma once
#include <SFML/Graphics.hpp>

namespace Clydesdale {
    namespace ECS {
        struct SpriteComponent {
            sf::Sprite* sprite;
            sf::Shader* shader = nullptr;

            SpriteComponent(const SpriteComponent&) = default;
            SpriteComponent(sf::Sprite* sprite) : sprite(sprite) {}
            SpriteComponent(sf::Sprite* sprite, sf::Shader* shader) : sprite(sprite), shader(shader) {}

            operator sf::Drawable& (){ return *sprite; }
            operator sf::Sprite& (){ return *sprite; }
            operator const sf::Sprite& (){ return *sprite; }
        };
    }
}