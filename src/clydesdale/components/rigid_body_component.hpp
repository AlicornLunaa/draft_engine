#pragma once
#include "clydesdale/phys/rigid_body.hpp"
#include "clydesdale/phys/world.hpp"

namespace Clydesdale {
    struct RigidBodyComponent : public RigidBody {
        // Constructors
        using RigidBody::RigidBody;

        RigidBodyComponent(const RigidBodyComponent&) = default;
        
        RigidBodyComponent(Clydesdale::World& world, BodyDef& bodyDef, float width, float height) {
            FixtureDef fixtureDef;
            PolygonShape dynamicBox;
            dynamicBox.setAsBox(width / 2.f, height / 2.f);
            fixtureDef.shape = dynamicBox;
            fixtureDef.density = 1.0f;
            fixtureDef.friction = 0.3f;

            setBody(world.createBody(bodyDef));
            createFixture(&fixtureDef);
        }
    };
}