#pragma once

#include "draft/math/glm.hpp"
#include "draft/phys/body_def.hpp"
#include "draft/phys/joint.hpp"
#include "draft/phys/joint_def.hpp"
#include "draft/phys/raycast_props.hpp"
#include "draft/phys/rigid_body.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/util/asset_manager/asset_manager.hpp"
#include "draft/util/time.hpp"

#include <memory>
#include <vector>

namespace Draft {
    class World {
    private:
        // Variables
        std::vector<std::unique_ptr<RigidBody>> rigidBodies;
        std::vector<std::unique_ptr<Joint>> joints;

    public:
        // Static public vars
        constexpr static int VELOCITY_ITER = 6;
        constexpr static int POSITION_ITER = 2;

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

        template<typename T>
        Joint* create_joint(const T& def);
        void destroy_joint(Joint* jointPtr);

        void set_debug_renderer(Resource<Shader> shader = Assets::manager.get<Shader>("assets/shaders/shapes", true), void* renderer = nullptr);
        void set_destruction_listener(void* listener) noexcept;
        void step(Time timeStep, int32_t velocityIterations, int32_t positionIterations);
        void debug_draw(const Matrix4& m = Matrix4(1.f));

        void raycast(RaycastCallback callback, const Vector2f& point1, const Vector2f& point2) const;
        
    private:
        // pImpl
        struct Impl;
        std::unique_ptr<Impl> ptr;
    };
};