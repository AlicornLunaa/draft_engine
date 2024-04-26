#pragma once

#include "draft/math/bounds.hpp"

namespace Draft {
    template<typename T>
    struct Rect {
        // Variables
        T x;
        T y;
        T width;
        T height;

        // Operator
        operator Bounds() const {
            return Bounds{{ x, y }, { x + width, y }, { x + width, y + height }, { x, y + height }};
        }
    };

    typedef Rect<int> IntRect;
    typedef Rect<float> FloatRect;
    typedef Rect<double> DoubleRect;
};

namespace glm {
    template<typename T>
    const Draft::Vector2<T> normalize_coordinates(const Draft::Rect<T>& rect, const Draft::Vector2<T>& coords){
        return {
            ((coords.x - rect.x) / rect.width - 0.5f) * 2.f,
            ((coords.y - rect.y) / rect.height - 0.5f) * 2.f
        };
    }
};