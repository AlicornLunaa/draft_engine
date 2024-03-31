#pragma once

#include "box2d/b2_math.h"
#include "SFML/System/Vector3.hpp"
#include "draft/math/vector3.hpp"

namespace Draft {
    template<typename T>
    inline b2Vec3 vector_to_b2(const Vector3<T>& v){ return b2Vec3(v.x, v.y, v.z); }

    template<typename T>
    inline Vector3<T> b2_to_vector(const b2Vec3& v){ return Vector2<T>(v.x, v.y, v.z); }

    template<typename T>
    inline sf::Vector3<T> vector_to_sf(const Vector3<T>& v){ return sf::Vector3<T>(v.x, v.y, v.z); }

    template<typename T>
    inline Vector3<T> sf_to_vector(const sf::Vector3<T>& v){ return Vector2<T>(v.x, v.y, v.z); }
};