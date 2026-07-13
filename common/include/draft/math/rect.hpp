#pragma once

#include "draft/math/bounds.hpp"
#include "draft/util/reflectable.hpp"

namespace Draft {
    /**
     * @brief An axis-aligned rectangle with position (x, y) plus size (width, height).
     */
    template<typename T>
    struct Rect {
        // Variables
        T x{};
        T y{};
        T width{};
        T height{};

        // Constructors
        Rect(T x = 0, T y = 0, T width = 0, T height = 0) : x(x), y(y), width(width), height(height) {}
        Rect(const Vector2<T>& pos, const Vector2<T>& size) : x(pos.x), y(pos.y), width(size.x), height(size.y) {}

        DRAFT_REFLECTABLE(Rect, x, y, width, height)

        /**
         * @brief Converts this rect to its four corners, wound clockwise from (x, y).
         */
        operator Bounds() const {
            return Bounds{{ x, y }, { x + width, y }, { x + width, y + height }, { x, y + height }};
        }

        template<typename U>
        operator Rect<U>(){
            Rect<U> rect;
            rect.x = (U)x;
            rect.y = (U)y;
            rect.width = (U)width;
            rect.height = (U)height;
            return rect;
        }
    };

    typedef Rect<unsigned int> UIntRect;
    typedef Rect<int> IntRect;
    typedef Rect<float> FloatRect;
    typedef Rect<double> DoubleRect;
};

namespace glm {
    /**
     * @brief Remaps @p coords from @p rect's local space into normalized device coordinates
     * ([-1, 1] on both axes, with y flipped so +y is up rather than down).
     */
    template<typename T>
    const Draft::Vector2<T> normalize_coordinates(const Draft::Rect<T>& rect, const Draft::Vector2<T>& coords){
        return {
            ((coords.x - rect.x) / rect.width - 0.5f) * 2.f,
            (1.f - (coords.y - rect.y) / rect.height - 0.5f) * 2.f
        };
    }
};
