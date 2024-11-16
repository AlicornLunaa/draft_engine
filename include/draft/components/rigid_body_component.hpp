#pragma once

#include "draft/phys/body_def.hpp"
#include "draft/phys/fixture.hpp"

namespace Draft {
    struct RigidBodyComponent {
        // Variables
        RigidBody* bodyPtr = nullptr;
        BodyDef bodyDef;

        // Constructors
        RigidBodyComponent() = default;
        RigidBodyComponent(const BodyDef& def) : bodyDef(def) {}

        // Operators
        operator RigidBody& () { return *bodyPtr; }
        operator RigidBody* () { return bodyPtr; }
        operator const RigidBody* () const { return bodyPtr; }
        operator bool () const { return is_valid(); }

        // Functions
        bool is_valid() const { return bodyPtr != nullptr; }
    };
}