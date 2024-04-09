#pragma once

#include <cmath>
#include <ostream>

namespace Draft {
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

        friend std::ostream& operator<< (std::ostream& stream, const Vector2<T>& v){
            stream << "(" << v.x << ", " << v.y << ")";
            return stream;
        }
    };

    typedef Vector2<float> Vector2f;
    typedef Vector2<double> Vector2d;
    typedef Vector2<int> Vector2i;
    typedef Vector2<unsigned int> Vector2u;
}