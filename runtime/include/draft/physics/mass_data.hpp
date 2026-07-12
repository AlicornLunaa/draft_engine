#pragma once

#include "draft/math/glm.hpp"
#include "draft/util/reflectable.hpp"

namespace Draft {
    /**
     * @brief Mass, center of mass, and rotational inertia of a rigid body
     */
    struct MassData {
        DRAFT_REFLECTED(float, mass) = 0.f;
        DRAFT_REFLECTED(Vector2f, centerOfMass) = {0, 0};
        DRAFT_REFLECTED(float, inertia) = 0.f;

        DRAFT_REFLECTABLE(MassData, mass, centerOfMass, inertia)
    };
}
