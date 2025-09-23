#pragma once

#include "draft/math/glm.hpp"

namespace Draft {
    struct MassData {
        float mass = 0.f;
        Vector2f centerOfMass = {0, 0};
        float inertia = 0.f;
    };
};