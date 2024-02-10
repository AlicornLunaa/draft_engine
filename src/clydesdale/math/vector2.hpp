#pragma once
#include <SFML/Graphics.hpp>
#include <box2d/b2_math.h>

namespace Clydesdale::Math {
    class Vector2f : public sf::Vector2f {
    public:
        using sf::Vector2f::Vector2f;
        Vector2f(const sf::Vector2f& other) : sf::Vector2f(other.x, other.y) {}
        Vector2f(const b2Vec2& other) : sf::Vector2f(other.x, other.y) {}

        operator b2Vec2 () { return b2Vec2(x, y); }
        explicit operator const b2Vec2 () { return b2Vec2(x, y); }
    };

    class Vector2i : public sf::Vector2i {
    public:
        using sf::Vector2i::Vector2i;
        Vector2i(const sf::Vector2i& other) : sf::Vector2i(other.x, other.y) {}
        Vector2i(const b2Vec2& other) : sf::Vector2i((int)other.x, (int)other.y) {}

        operator b2Vec2 () { return b2Vec2(x, y); }
        explicit operator const b2Vec2 () { return b2Vec2(x, y); }
    };

    class Vector2u : public sf::Vector2u {
    public:
        using sf::Vector2u::Vector2u;
        Vector2u(const sf::Vector2u& other) : sf::Vector2u(other.x, other.y) {}
        Vector2u(const b2Vec2& other) : sf::Vector2u((unsigned int)other.x, (unsigned int)other.y) {}

        operator b2Vec2 () { return b2Vec2(x, y); }
        explicit operator const b2Vec2 () { return b2Vec2(x, y); }
    };
}