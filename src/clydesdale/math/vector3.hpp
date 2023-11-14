#pragma once
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

namespace Clydesdale::Math {
    class Vector3f : public sf::Vector3f {
        using sf::Vector3f::Vector3f;

        operator b2Vec3 () { return b2Vec3(x, y, z); }
        operator const b2Vec3 () { return b2Vec3(x, y, z); }
    };

    class Vector3i : public sf::Vector3i {
        using sf::Vector3i::Vector3i;

        operator b2Vec3 () { return b2Vec3(x, y, z); }
        operator const b2Vec3 () { return b2Vec3(x, y, z); }
    };
}