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