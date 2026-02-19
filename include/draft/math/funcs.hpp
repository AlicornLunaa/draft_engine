#pragma once

#include "glm/common.hpp"

namespace glm {
    template<typename T, typename U>
    T mod_floor(T a, U n){
        return ((a % n) + n) % n;
    }

    template<typename T, typename U>
    T div_floor(T a, U n){
        return glm::floor(a / n);
    }

    template<typename T>
    T cross(const glm::vec<2, T>& a, const glm::vec<2, T>& b){
        return a.x * b.y - a.y * b.x;
    }

    template<typename T>
    T lerp(const T& a, const T& b, float t){
        return a * (1 - t) + b * t;
    }

    template<typename T>
    glm::vec<2, T> perpendicular(const glm::vec<2, T>& v){
        return {-v.y, v.x};
    }
};