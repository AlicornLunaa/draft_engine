#pragma once

#include <cmath>
#include <ostream>

namespace Draft {
    // Primary template
    template<typename T>
    class Vector3 {
    public:
        // Variables
        T x;
        T y;
        T z;

        // Constructor
        Vector3() : x(), y(), z() {};
        Vector3(T x, T y, T z) : x(x), y(y), z(z) {}

        template<typename U>
        Vector3(const Vector3<U>& other) : x((T)other.x), y((T)other.y), z((T)other.z) {}

        // Functions
        inline T lengthSqr(){ return (x * x + y * y + z * z); }
        inline T length(){ return std::sqrt(lengthSqr()); }
        inline Vector3<T> normalized(){ return Vector3(this) / length(); }
        inline Vector3<T> cross(const Vector3<T>& other){ return { y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x }; }

        // Operators
        Vector3<T> operator- (){ return { -x, -y, -z }; }
        Vector3<T>& operator+= (const Vector3<T> &r){ x += r.x; y += r.y; z += r.z; return this; }
        Vector3<T>& operator-= (const Vector3<T> &r){ x -= r.x; y -= r.y; z -= r.z; return this; }
        Vector3<T>& operator*= (T r){ x *= r; y *= r; z *= r; return this; }
        Vector3<T>& operator/= (T r){ x /= r; y /= r; z /= r; return this; }
        Vector3<T> operator+ (const Vector3<T> &r){ return { x + r.x, y + r.y, z + r.z }; }
        Vector3<T> operator- (const Vector3<T> &r){ return { x - r.x, y - r.y, z - r.z }; }
        Vector3<T> operator* (T r){ return { x * r, y * r, z * r }; }
        Vector3<T> operator/ (T r){ return { x / r, y / r, z / r }; }
        T operator* (const Vector3<T> &r){ return (x * r.x + y * r.y + z * r.z); }
        bool operator== (const Vector3<T> &r){ return (x == r.x && y == r.y && z == r.z); }
        bool operator!= (const Vector3<T> &r){ return (x != r.x || y != r.y || z != r.z); }

        friend std::ostream& operator<< (std::ostream& stream, const Vector3<T>& v){
            stream << "(" << v.x << ", " << v.y << ", " << v.z << ")";
            return stream;
        }
    };

    // Aliases
    typedef Vector3<float> Vector3f;
    typedef Vector3<double> Vector3d;
    typedef Vector3<int> Vector3i;
    typedef Vector3<unsigned int> Vector3u;
}