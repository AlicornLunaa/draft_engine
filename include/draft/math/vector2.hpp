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
        /**
         * @brief Returns the length of the vector squared
         * @return T 
         */
        inline T lengthSqr() const { return (x * x + y * y); }
        
        /**
         * @brief Returns the length of the vector. Slower than a squared length!
         * @return T 
         */
        inline T length() const { return std::sqrt(lengthSqr()); }

        /**
         * @brief Returns a copy of the normalized vector
         * @return Vector2<T> 
         */
        inline Vector2<T> normalized() const { return Vector2(this) / length(); }

        /**
         * @brief Returns the theoretical 2D cross product
         * @param other 
         * @return T 
         */
        inline T cross(const Vector2<T>& other) const { return x * other.x - y * other.x; }

        /**
         * @brief Sets X and Y
         * @param x 
         * @param y 
         */
        inline void set(T x, T y){ this->x = x; this->y = y; }

        // Operators
        Vector2<T> operator- () const { return { -x, -y }; }
        Vector2<T>& operator+= (const Vector2<T> &r){ x += r.x; y += r.y; return this; }
        Vector2<T>& operator-= (const Vector2<T> &r){ x -= r.x; y -= r.y; return this; }
        Vector2<T>& operator*= (T r){ x *= r; y *= r; return this; }
        Vector2<T>& operator/= (T r){ x /= r; y /= r; return this; }
        Vector2<T> operator+ (const Vector2<T> &r) const { return { x + r.x, y + r.y }; }
        Vector2<T> operator- (const Vector2<T> &r) const { return { x - r.x, y - r.y }; }
        Vector2<T> operator* (T r) const { return { x * r, y * r }; }
        Vector2<T> operator/ (T r) const { return { x / r, y / r }; }
        T operator* (const Vector2<T> &r) const { return (x * r.x + y * r.y); }
        bool operator== (const Vector2<T> &r) const { return (x == r.x && y == r.y); }
        bool operator!= (const Vector2<T> &r) const { return (x != r.x || y != r.y); }

        operator Vector3<T>() const { return { x, y, (T)0 }; }
        operator Vector4<T>() const { return { x, y, (T)0, (T)0 }; }

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