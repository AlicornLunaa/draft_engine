#pragma once

#include "draft/math/glm.hpp"

namespace Draft {
    struct Vertex {
        Vector2f position{0, 0};
        Vector2f texCoord{0, 0};
        Vector4f color{1, 1, 1, 1};
    };
};