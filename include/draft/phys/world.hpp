#pragma once

#include "draft/math/vector2.hpp"
#include "draft/phys/body_def.hpp"
#include "draft/phys/joint.hpp"
#include "draft/phys/joint_def.hpp"
#include "draft/phys/rigid_body.hpp"

#include <memory>
#include <vector>

namespace Draft {
    class World {
    private:
        // Variables
        std::vector<RigidBody*> rigidBodies;
        std::vector<Joint*> joints;

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
        RigidBody* create_rigid_body(const BodyDef& def);
        void destroy_body(RigidBody*& rigidBody);
        void destroy_body(RigidBody* rigidBody);

        template<typename T>
        void create_joint(const T& def);
        void destroy_joint(Joint*& joint);
        void destroy_joint(Joint* joint);

        void set_destruction_listener(void* listener) noexcept;
        void step(float timeStep, int32_t velocityIterations, int32_t positionIterations);
        
    private:
        // pImpl
        struct Impl;
        std::unique_ptr<Impl> ptr;
    };
};