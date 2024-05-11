#pragma once

#include "draft/math/glm.hpp"
#include "draft/phys/body_def.hpp"
#include "draft/phys/joint.hpp"
#include "draft/phys/joint_def.hpp"
#include "draft/phys/rigid_body.hpp"
#include "draft/rendering/camera.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/util/asset_manager.hpp"
#include "draft/util/time.hpp"

#include <memory>
#include <vector>

namespace Draft {
    class World {
    private:
        // Variables
        std::vector<RigidBody*> rigidBodies;
        std::vector<Joint*> joints;

    public:
        // Static public vars
        constexpr static int VELOCITY_ITER = 6;
        constexpr static int POSITION_ITER = 2;

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
        Joint* create_joint(const T& def);
        void destroy_joint(Joint*& joint);
        void destroy_joint(Joint* joint);

        void set_debug_renderer(const Shader& shader = Assets::get_asset<Shader>("assets/shaders/shapes"), void* renderer = nullptr);
        void set_destruction_listener(void* listener) noexcept;
        void step(Time timeStep, int32_t velocityIterations, int32_t positionIterations);
        void debug_draw(const RenderWindow& window, const Camera* camera = nullptr);
        
    private:
        // pImpl
        struct Impl;
        std::unique_ptr<Impl> ptr;
    };
};