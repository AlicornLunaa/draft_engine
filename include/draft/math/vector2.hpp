#pragma once

#include <cmath>
#include <ostream>

#include "draft/math/vector3.hpp"
#include "draft/math/vector4.hpp"

namespace Draft {
    // Primary template
    template<typename T>
    class Vector2 {
    public:
        // Variables
        T x;
        T y;

        // Constructor
        Vector2() : x(), y() {};
        Vector2(T x, T y) : x(x), y(y) {}

        template<typename U>
        Vector2(const Vector2<U>& other) : x((T)other.x), y((T)other.y) {}

        // Functions
        inline T lengthSqr(){ return (x * x + y * y); }
        inline T length(){ return std::sqrt(lengthSqr()); }
        inline Vector2<T> normalized(){ return Vector2(this) / length(); }
        inline T cross(const Vector2<T>& other){ return x * other.x - y * other.x; }

        // Operators
        Vector2<T> operator- (){ return { -x, -y }; }
        Vector2<T>& operator+= (const Vector2<T> &r){ x += r.x; y += r.y; return this; }
        Vector2<T>& operator-= (const Vector2<T> &r){ x -= r.x; y -= r.y; return this; }
        Vector2<T>& operator*= (T r){ x *= r; y *= r; return this; }
        Vector2<T>& operator/= (T r){ x /= r; y /= r; return this; }
        Vector2<T> operator+ (const Vector2<T> &r){ return { x + r.x, y + r.y }; }
        Vector2<T> operator- (const Vector2<T> &r){ return { x - r.x, y - r.y }; }
        Vector2<T> operator* (T r){ return { x * r, y * r }; }
        Vector2<T> operator/ (T r){ return { x / r, y / r }; }
        T operator* (const Vector2<T> &r){ return (x * r.x + y * r.y); }
        bool operator== (const Vector2<T> &r){ return (x == r.x && y == r.y); }
        bool operator!= (const Vector2<T> &r){ return (x != r.x || y != r.y); }

        operator Vector3<T>(){ return { x, y, (T)0 }; }
        operator Vector4<T>(){ return { x, y, (T)0, (T)0 }; }

        friend std::ostream& operator<< (std::ostream& stream, const Vector2<T>& v){
            stream << "(" << v.x << ", " << v.y << ")";
            return stream;
        }
    };

    // Aliases
    typedef Vector2<float> Vector2f;
    typedef Vector2<double> Vector2d;
    typedef Vector2<int> Vector2i;
    typedef Vector2<unsigned int> Vector2u;
}