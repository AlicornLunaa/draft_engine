#pragma once

#include "glm/common.hpp"

namespace glm {
    /**
     * @brief Floored modulo of @p a by @p n (result always has the same sign as @p n),
     * unlike `operator%`, whose result takes the sign of @p a.
     */
    template<typename T, typename U>
    T mod_floor(T a, U n){
        return ((a % n) + n) % n;
    }

    /**
     * @brief Floored division of @p a by @p n, i.e. `floor(a / n)`.
     */
    template<typename T, typename U>
    T div_floor(T a, U n){
        return glm::floor(a / n);
    }

    /**
     * @brief 2D cross product (the scalar z-component of the 3D cross product of @p a and @p b
     * extended into the xy-plane).
     */
    template<typename T>
    T cross(const glm::vec<2, T>& a, const glm::vec<2, T>& b){
        return a.x * b.y - a.y * b.x;
    }

    /**
     * @brief Linear interpolation between @p a and @p b by fraction @p t.
     */
    template<typename T>
    T lerp(const T& a, const T& b, float t){
        return a * (1 - t) + b * t;
    }

    /**
     * @brief Remaps @p value from [@p inMin, @p inMax] to [@p outMin, @p outMax], linearly.
     * Doesn't clamp, a @p value outside [@p inMin, @p inMax] extrapolates past
     * [@p outMin, @p outMax] the same way, callers that want clamping should glm::clamp() the
     * result themselves.
     */
    template<typename T>
    T map(T value, T inMin, T inMax, T outMin, T outMax){
        return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin);
    }

    /**
     * @brief Inverse of map(): given @p value already in [@p rangeMin, @p rangeMax], returns
     * where it falls within [0, 1] - equivalent to map(value, rangeMin, rangeMax, 0, 1). Useful
     * for turning a concrete value back into the fraction lerp()/glm::slerp() take as @p t.
     */
    template<typename T>
    T inverse_lerp(T value, T rangeMin, T rangeMax){
        return (value - rangeMin) / (rangeMax - rangeMin);
    }

    /**
     * @brief A vector perpendicular to @p v (rotated 90 degrees counter-clockwise).
     */
    template<typename T>
    glm::vec<2, T> perpendicular(const glm::vec<2, T>& v){
        return {-v.y, v.x};
    }
};
