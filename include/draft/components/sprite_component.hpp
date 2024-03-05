#pragma once

#include "draft/math/vector2.hpp"
#include <SFML/Graphics.hpp>

namespace Draft {
    struct SpriteComponent {
        // Variables
        sf::Shader* shader = nullptr;

        Vector2f position = { -0.5f, -0.5f };
        Vector2f size = { 1, 1 };
        float rotation = 0.f;
        
        unsigned short textureUnit = 0; // Offset for which texture to use

        // Constructors
        SpriteComponent(const SpriteComponent&) = default;
        SpriteComponent(sf::Shader* shader) : shader(shader) {}
        SpriteComponent(Vector2f position = { 0, 0 }, Vector2f size = { 1, 1 }, float rotation = 0.f, sf::Shader* shader = nullptr)
            : shader(shader), position(position), size(size), rotation(rotation) {}
    };
}