#pragma once
#include <SFML/Graphics.hpp>
#include <box2d/b2_math.h>

namespace Clydesdale::Math {
    class Vector3f : public sf::Vector3f {
    public:
        using sf::Vector3f::Vector3f;
        Vector3f(const sf::Vector3f& other) : sf::Vector3f(other.x, other.y, other.z) {}
        Vector3f(const b2Vec3& other) : sf::Vector3f(other.x, other.y, other.z) {}

        operator b2Vec3 () { return b2Vec3(x, y, z); }
        operator const b2Vec3 () { return b2Vec3(x, y, z); }
    };

    class Vector3i : public sf::Vector3i {
    public:
        using sf::Vector3i::Vector3i;
        Vector3i(const sf::Vector3i& other) : sf::Vector3i(other.x, other.y, other.z) {}
        Vector3i(const b2Vec3& other) : sf::Vector3i((int)other.x, (int)other.y, (int)other.z) {}

        operator b2Vec3 () { return b2Vec3(x, y, z); }
        operator const b2Vec3 () { return b2Vec3(x, y, z); }
    };
}