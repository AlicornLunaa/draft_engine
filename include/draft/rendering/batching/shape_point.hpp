#pragma once

#include "draft/math/glm.hpp"

namespace Draft {
    struct ShapePoint {
        Vector2f position{0, 0};
        Vector4f color{1, 1, 1, 1};
    };

    enum class ShapeRenderType { FILL, LINE };
}