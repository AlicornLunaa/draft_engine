#pragma once

#include <cmath>
#include <ostream>

namespace Draft {
    // Primary template
    template<typename T>
    class Vector4 {
    public:
        // Variables
        T x;
        T y;
        T z;
        T w;

        // Constructor
        Vector4() : x(), y(), z(), w() {};
        Vector4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

        template<typename U>
        Vector4(const Vector4<U>& other) : x((T)other.x), y((T)other.y), z((T)other.z), w((T)other.w) {}

        // Functions
        inline T lengthSqr() const { return (x * x + y * y + z * z + w * w); }
        inline T length() const { return std::sqrt(lengthSqr()); }
        inline Vector4<T> normalized() const { return Vector4(this) / length(); }
        inline void set(T x, T y, T z, T w){ this->x = x; this->y = y; this->z = z; this->w = w; }

        // Operators
        Vector4<T> operator- () const { return { -x, -y, -z, -w }; }
        Vector4<T>& operator+= (const Vector4<T> &r){ x += r.x; y += r.y; z += r.z; w += r.w; return this; }
        Vector4<T>& operator-= (const Vector4<T> &r){ x -= r.x; y -= r.y; z -= r.z; w -= r.w; return this; }
        Vector4<T>& operator*= (T r){ x *= r; y *= r; z *= r; w *= r; return this; }
        Vector4<T>& operator/= (T r){ x /= r; y /= r; z /= r; w /= r; return this; }
        Vector4<T> operator+ (const Vector4<T> &r) const { return { x + r.x, y + r.y, z + r.z, w + r.w }; }
        Vector4<T> operator- (const Vector4<T> &r) const { return { x - r.x, y - r.y, z - r.z, w - r.w }; }
        Vector4<T> operator* (T r) const { return { x * r, y * r, z * r, w * r }; }
        Vector4<T> operator/ (T r) const { return { x / r, y / r, z / r, w / r }; }
        T operator* (const Vector4<T> &r) const { return (x * r.x + y * r.y + z * r.z + w * r.w); }
        bool operator== (const Vector4<T> &r) const { return (x == r.x && y == r.y && z == r.z && w == r.w); }
        bool operator!= (const Vector4<T> &r) const { return (x != r.x || y != r.y || z != r.z || w != r.w); }

        friend std::ostream& operator<< (std::ostream& stream, const Vector4<T>& v){
            stream << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
            return stream;
        }
    };

    // Aliases
    typedef Vector4<float> Vector4f;
    typedef Vector4<double> Vector4d;
    typedef Vector4<int> Vector4i;
    typedef Vector4<unsigned int> Vector4u;
}