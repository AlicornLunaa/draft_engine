#pragma once

#include "draft/phys/body_def.hpp"
#include "draft/phys/fixture.hpp"
#include "draft/phys/world.hpp"

namespace Draft {
    struct RigidBodyComponent {
        // Variables
        RigidBody* ptr = nullptr;

        // Constructors
        RigidBodyComponent(World* world, const BodyDef& def) : ptr(world->create_rigid_body(def)) {}

        // Operators
        operator RigidBody& () { return *ptr; }
        operator RigidBody* () { return ptr; }
        operator const RigidBody* () const { return ptr; }
    };
}