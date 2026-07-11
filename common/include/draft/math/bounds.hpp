#pragma once

#include "draft/math/glm.hpp"
#include <vector>

namespace Draft {
    /**
     * @brief An ordered polygon outline, each point connects to the next and wraps back to
     * the first.
     */
    typedef std::vector<Vector2f> Bounds;
};

namespace glm {
    /**
     * @brief Checks whether @p point lies inside (or exactly on the edge of) the polygon
     * described by @p bounds, via a wrapping ray cast. Works for convex and concave polygons.
     */
    bool contains(const Draft::Bounds& bounds, const Draft::Vector2f& point);
};
