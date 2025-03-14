#pragma once

#include "draft/math/bounds.hpp"
#include "draft/rendering/render_window.hpp"

namespace Draft {
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

        // Operator
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
    template<typename T>
    const Draft::Vector2<T> normalize_coordinates(const Draft::Rect<T>& rect, const Draft::Vector2<T>& coords){
        return {
            ((coords.x - rect.x) / rect.width - 0.5f) * 2.f,
            (1.f - (coords.y - rect.y) / rect.height - 0.5f) * 2.f
        };
    }

    template<typename T>
    const Draft::Vector2<T> normalize_coordinates(const Draft::RenderWindow& window, const Draft::Vector2<T>& coords){
        auto& size = window.get_size();
        Draft::UIntRect rect{0, 0, size.x, size.y};

        return {
            ((coords.x - rect.x) / rect.width - 0.5f) * 2.f,
            (1.f - (coords.y - rect.y) / rect.height - 0.5f) * 2.f
        };
    }
};