#pragma once

#include "draft/math/glm.hpp"
#include "draft/phys/body_def.hpp"
#include "draft/phys/joint.hpp"
#include "draft/phys/joint_def.hpp"
#include "draft/phys/raycast_props.hpp"
#include "draft/phys/rigid_body.hpp"
#include "draft/rendering/pipeline/renderer.hpp"
#include "draft/util/time.hpp"

#include <memory>
#include <vector>

namespace Draft {
    class World {
    private:
        // Variables
        std::vector<std::unique_ptr<RigidBody>> rigidBodies;
        std::vector<std::unique_ptr<Joint>> joints;
        Vector2d offsetShift{};

    public:
        // Static public vars
        constexpr static int VELOCITY_ITER = 10;
        constexpr static int POSITION_ITER = 5;

        // Constructors
        World(const Vector2f& gravity);
        World(const World& other) = delete;
        ~World();

        // Operators
        World& operator=(const World& other) = delete;

        // Friends
        friend class RigidBody;

        // Functions
        RigidBody* create_rigid_body(const BodyDef& def);
        RigidBody* get_body(void* ptr) const;
        void destroy_body(RigidBody* rigidBodyPtr);
        size_t get_body_count() const;

        Joint* create_joint(const JointDef& def);
        void destroy_joint(Joint* jointPtr);

        Vector2f get_gravity() const;
        void set_gravity(const Vector2f& v);
        void shift_origin(const Vector2d& shift);
        inline const Vector2d& get_shift_offset() const { return offsetShift; }

        void step(Time timeStep, int32_t velocityIterations, int32_t positionIterations);

        void raycast(RaycastCallback callback, const Vector2f& point1, const Vector2f& point2) const;
        RigidBody* test_point(const Vector2f& position) const;

        void set_debug_renderer();
        void debug_draw(Renderer& renderer, const Matrix4& m = Matrix4(1.f));

    private:
        // pImpl
        struct Impl;
        std::unique_ptr<Impl> ptr;
    };
};
