#include "b2_raycast_proxy_p.hpp"
#include "draft/math/vector2_p.hpp"
#include "draft/phys/rigid_body.hpp"
#include "box2d/b2_fixture.h"

namespace Draft {
    // Constructors
    B2RaycastProxy::B2RaycastProxy(const World& world, RaycastCallback& callback) : world(world), callback(callback) {};

    // Functions
    float B2RaycastProxy::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction){
        // Convert to draft
        RigidBody* body = world.get_body(fixture->GetBody());
        Fixture* dFixture = body->get_fixture(fixture);

        return callback(dFixture, b2_to_vector<float>(point), b2_to_vector<float>(normal), fraction);
    }
};