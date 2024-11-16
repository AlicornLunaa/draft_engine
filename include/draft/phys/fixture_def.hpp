#pragma once

#include "draft/phys/shapes/shape.hpp"
#include "draft/phys/filter.hpp"

namespace Draft {
    struct FixtureDef {
        Shape* shape = nullptr;
        float friction = 0.2f;
        float restitution = 0.f;
        float restitutionThreshold = 1.f;
        float density = 0.f;
        bool isSensor = false;
        PhysMask filter;
    };
};