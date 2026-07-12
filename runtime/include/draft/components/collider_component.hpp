#pragma once

#include "draft/physics/collider.hpp"
#include "draft/physics/shapes/shape.hpp"

namespace Draft {
    /**
     * @brief Owns a `Collider`. `PhysicsSystem` attaches it to the entity's `NativeBodyComponent`
     * when both are present.
     */
    struct ColliderComponent {
        // Variables
        Collider collider;

        // Constructors
        ColliderComponent() = default;
        ColliderComponent(const Shape& shape){ collider.add_shape(shape); }
        ColliderComponent(const Collider& collider){ this->collider = collider; }

        // Operators
        Collider* operator->(){ return &collider; }
        operator Collider& () { return collider; }
        operator const Collider& () const { return collider; }

        DRAFT_REFLECTABLE(ColliderComponent, collider)
    };
}
