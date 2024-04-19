#pragma once

#include "draft/math/vector2.hpp"
#include "draft/phys/rigid_body.hpp"

namespace Draft {
    struct BodyDef {
        RigidBody::BodyType type = RigidBody::STATIC;
        Vector2f position = { 0, 0 };
        float angle = 0.f;

        Vector2f linearVelocity = { 0, 0 };
        float angularVelocity = 0.f;

        float linearDamping = 0.f;
        float angularDamping = 0.f;

        bool allowSleep = true;
        bool awake = true;

        bool fixedRotation = false;
        bool bullet = false;

        bool enabled = true;

        float gravityScale = 1.f;
    };
};