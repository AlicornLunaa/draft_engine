#pragma once

#include "draft/math/glm.hpp"
#include <vector>

namespace Draft {
    typedef std::vector<Vector2f> Bounds;
};

namespace glm {
    // Used to extend Math namespace since Math is just an alias
    bool contains(const Draft::Bounds& bounds, const Draft::Vector2f& point);
};