#pragma once

#include "draft/phys/body_def.hpp"
#include "draft/phys/fixture.hpp"
#include "draft/phys/world.hpp"

namespace Draft {
    struct RigidBodyComponent {
        // Variables
        RigidBody* bodyPtr = nullptr;

        // Constructors
        RigidBodyComponent(World* world, const BodyDef& def) : bodyPtr(world->create_rigid_body(def)) {}

        // Operators
        operator RigidBody& () { return *bodyPtr; }
        operator RigidBody* () { return bodyPtr; }
        operator const RigidBody* () const { return bodyPtr; }
    };
}