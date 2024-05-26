#pragma once

#include "draft/phys/collider.hpp"

namespace Draft {
    struct ColliderComponent {
        // Variables
        Collider collider;

        // Operators
        operator Collider& () { return collider; }
        operator const Collider& () const { return collider; }
    };
}