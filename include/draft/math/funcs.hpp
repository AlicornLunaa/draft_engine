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
};