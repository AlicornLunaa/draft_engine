#include "draft/phys/world.hpp"

#include "box2d/b2_body.h"
#include "box2d/b2_world.h"
#include "draft/math/vector2_p.hpp"
#include "draft/phys/conversions_p.hpp"
#include "draft/phys/rigid_body.hpp"

#include <algorithm>
#include <memory>

namespace Draft {
    // pImpl
    struct World::Impl {
        b2World world = b2World({0, 0});
    };

    // Constructors
    World::World(const Vector2f& gravity) : ptr(std::make_unique<Impl>()) {
        ptr->world.SetGravity(vector_to_b2(gravity));
    }

    World::~World(){
        for(RigidBody* rb : rigidBodies){
            if(!rb) continue;
            delete rb;
        }
    }

    // Functions
    RigidBody* World::create_rigid_body(const BodyDef& def){
        b2BodyDef tmp = bodydef_to_b2(def);
        b2Body* body = ptr->world.CreateBody(&tmp);
        RigidBody* rb = new RigidBody(this, body);
        rigidBodies.push_back(rb);
        return rb;
    }

    void World::destroy_body(RigidBody*& rigidBody){
        destroy_body(reinterpret_cast<RigidBody*>(rigidBody));
        rigidBody = nullptr;
    }

    void World::destroy_body(RigidBody* rigidBody){
        ptr->world.DestroyBody((b2Body*)rigidBody->get_body_ptr());
        rigidBodies.erase(std::find(rigidBodies.begin(), rigidBodies.end(), rigidBody));
    }

    void World::step(float timeStep, int32_t velocityIterations, int32_t positionIterations){ ptr->world.Step(timeStep, velocityIterations, positionIterations); }
};