#pragma once
#include <SFML/Graphics.hpp>
#include <box2d/b2_math.h>

namespace Clydesdale::Math {
    class Transform : public sf::Transform {
    public:
        using sf::Transform::Transform;
        Transform(const sf::Transform& other) : sf::Transform(other) {}
        Transform(const b2Mat33& m) : sf::Transform(m.ex.x, m.ex.y, m.ex.z, m.ey.x, m.ey.y, m.ey.z, m.ez.x, m.ez.y, m.ez.z) {}

        static const Transform Identity;

        operator b2Mat33 () {
            const float* ptr = getMatrix();
            return b2Mat33(b2Vec3(ptr[0], ptr[1], ptr[2]), b2Vec3(ptr[4], ptr[5], ptr[6]), b2Vec3(ptr[8], ptr[9], ptr[10]));
        }

        operator const b2Mat33 () {
            const float* ptr = getMatrix();
            return b2Mat33(b2Vec3(ptr[0], ptr[1], ptr[2]), b2Vec3(ptr[4], ptr[5], ptr[6]), b2Vec3(ptr[8], ptr[9], ptr[10]));
        }
    };
}