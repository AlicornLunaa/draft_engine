#pragma once

#include "box2d/b2_math.h"
#include "SFML/System/Vector2.hpp"
#include "draft/math/vector2.hpp"

namespace Draft {
    template<typename T>
    inline b2Vec2 vector_to_b2(const Vector2<T>& v){ return b2Vec2(v.x, v.y); }

    template<typename T>
    inline Vector2<T> b2_to_vector(const b2Vec2& v){ return Vector2<T>(v.x, v.y); }

    template<typename T>
    inline sf::Vector2<T> vector_to_sf(const Vector2<T>& v){ return sf::Vector2<T>(v.x, v.y); }

    template<typename T>
    inline Vector2<T> sf_to_vector(const sf::Vector2<T>& v){ return Vector2<T>(v.x, v.y); }
};