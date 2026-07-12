#pragma once

#include "draft/math/glm.hpp"
#include "draft/physics/body_type.hpp"

namespace Draft {
    struct BodyDef {
        BodyType type = BodyType::STATIC;
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
