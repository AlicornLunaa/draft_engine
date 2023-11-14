#pragma once
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

namespace Clydesdale::Math {
    class Vector2f : public sf::Vector2f {
        using sf::Vector2f::Vector2f;

        operator b2Vec2 () { return b2Vec2(x, y); }
        operator const b2Vec2 () { return b2Vec2(x, y); }
    };

    class Vector2i : public sf::Vector2i {
        using sf::Vector2i::Vector2i;

        operator b2Vec2 () { return b2Vec2(x, y); }
        operator const b2Vec2 () { return b2Vec2(x, y); }
    };

    class Vector2u : public sf::Vector2u {
        using sf::Vector2u::Vector2u;

        operator b2Vec2 () { return b2Vec2(x, y); }
        operator const b2Vec2 () { return b2Vec2(x, y); }
    };
}