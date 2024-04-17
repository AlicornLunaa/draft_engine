#pragma once

#include "box2d/b2_body.h"
#include "draft/math/vector2.hpp"
#include "draft/phys/rigid_body.hpp"

#include <memory>
#include <vector>

namespace Draft {
    class World {
    private:
        // Variables
        std::vector<RigidBody*> rigidBodies;

    public:
        // Constructors
        World(const Vector2f& gravity);
        World(const World& other) = delete;
        ~World();

        // Operators
        World(World&& other) noexcept = delete;
        World& operator=(const World& other) = delete;
        World& operator=(World&& other) noexcept = delete;

        // Functions
        RigidBody* create_rigid_body(const b2BodyDef* def);
        void destroy_body(RigidBody*& rigidBody);

        void set_destruction_listener(void* listener) noexcept;
        void step(float timeStep, int32_t velocityIterations, int32_t positionIterations);
        
    private:
        // pImpl
        struct Impl;
        std::unique_ptr<Impl> ptr;
    };
};