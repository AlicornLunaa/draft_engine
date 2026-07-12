#pragma once

#include "draft/phys/filter.hpp"

namespace Draft {
    class Shape;

    /**
     * @brief Blueprint for attaching a shape to a rigid body
     */
    struct FixtureDef {
        Shape* shape = nullptr;
        float friction = 0.2f;
        float restitution = 0.f;
        float restitutionThreshold = 1.f;
        float density = 0.f;
        bool isSensor = false;
        PhysMask filter;
    };
}
