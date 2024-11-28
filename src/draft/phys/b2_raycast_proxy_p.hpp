#pragma once

#include "box2d/b2_world_callbacks.h"
#include "draft/phys/raycast_props.hpp"
#include "draft/phys/world.hpp"

namespace Draft {
    class B2RaycastProxy : public b2RayCastCallback {
    private:
        // Variables
        const World& world;
        RaycastCallback& callback;

    public:
        // Constructors
        B2RaycastProxy(const World& world, RaycastCallback& callback);
        ~B2RaycastProxy() = default;

        // Functions
        virtual float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction);
    };
};