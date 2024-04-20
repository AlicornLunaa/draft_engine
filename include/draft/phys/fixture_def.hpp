#pragma once

#include "draft/phys/shape.hpp"
#include "draft/phys/filter.hpp"

namespace Draft {
    struct FixtureDef {
        Shape* shape;
        float friction;
        float restitution;
        float restitutionThreshold;
        float density;
        bool isSensor;
        Filter filter;
    };
};