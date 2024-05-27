#pragma once

#include "draft/phys/collider.hpp"
#include "draft/phys/shapes/shape.hpp"

namespace Draft {
    struct ColliderComponent {
        // Variables
        Collider collider;

        // Constructors
        ColliderComponent() = default;
        ColliderComponent(const Shape& shape){ collider.add_shape(shape); }

        // Operators
        operator Collider& () { return collider; }
        operator const Collider& () const { return collider; }
    };
}