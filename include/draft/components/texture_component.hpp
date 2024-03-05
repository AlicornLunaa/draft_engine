#pragma once

#include "SFML/Graphics/Texture.hpp"
#include <SFML/Graphics.hpp>

namespace Draft {
    struct TextureComponent {
        // Variables
        sf::Texture texture;

        // Constructors
        TextureComponent(const TextureComponent&) = default;
        TextureComponent(const sf::Texture& texture) : texture(texture) {}

        // Operators
        operator sf::Texture& () { return texture; }
        operator sf::Texture* () { return &texture; }
        explicit operator const sf::Texture& () { return texture; }
        explicit operator const sf::Texture* () { return &texture; }
    };
}