#pragma once

#include "draft/math/glm.hpp"
#include "draft/phys/fixture.hpp"
#include "draft/phys/rigid_body.hpp"

namespace Draft {
    /// RigidBody pointer for physics
    struct NativeBodyComponent {
        RigidBody* bodyPtr = nullptr;
        Vector2f deltaP = {};
        float deltaR = 0.f;

        RigidBody* operator->(){ return bodyPtr; }
        operator RigidBody& () { return *bodyPtr; }
        operator RigidBody* () { return bodyPtr; }
        operator const RigidBody* () const { return bodyPtr; }
        operator bool () const { return is_valid(); }

        bool is_valid() const { return bodyPtr != nullptr && bodyPtr->is_valid(); }
    };

    /// Abstracted rigidbody component
    struct RigidBodyComponent {
        RigidBody::BodyType type = RigidBody::STATIC;

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

    /// Forces components
    struct TorqueComponent {
        float torque = 0.f;
        bool wake = true;
    };

    struct ForceComponent {
        Vector2f force;
        Vector2f point;
        bool wake = true;
    };

    struct ImpulseComponent {
        Vector2f force;
        Vector2f point;
        float angular = 0.f;
        bool wake = true;
    };

    struct ContinuousTorqueComponent {
        float torque = 0.f;
        bool wake = true;
        bool enabled = false;
    };

    struct ContinuousForceComponent {
        Vector2f force;
        Vector2f point;
        bool wake = true;
        bool enabled = true;
    };

    struct ContinuousImpulseComponent {
        Vector2f force;
        Vector2f point;
        float angular = 0.f;
        bool wake = true;
        bool enabled = true;
    };
}