#pragma once

#include "draft/core/application.hpp"
#include "draft/core/registry.hpp"
#include "draft/core/scene.hpp"
#include "draft/phys/world.hpp"

namespace Draft {
    class PhysicsSystem {
    private:
        // Variables
        Application* m_appPtr = nullptr;
        Registry& m_registryRef;
        Scene& m_sceneRef;

        // Private functions
        void construct_body_func(Registry& reg, entt::entity rawEnt);
        void construct_native_body_func(Registry& reg, entt::entity rawEnt);
        void construct_collider_func(Registry& reg, entt::entity rawEnt);
        void construct_joint_func(Registry& reg, entt::entity rawEnt);
        void construct_native_joint_func(Registry& reg, entt::entity rawEnt);
        
        void deconstruct_body_func(Registry& reg, entt::entity rawEnt);
        void deconstruct_native_body_func(Registry& reg, entt::entity rawEnt);
        void deconstruct_collider_func(Registry& reg, entt::entity rawEnt);
        void deconstruct_joint_func(Registry& reg, entt::entity rawEnt);
        void deconstruct_native_joint_func(Registry& reg, entt::entity rawEnt);

        void handle_bodies();
        void handle_forces();

    public:
        // Public variables
        Time physicsTimestep = Time::seconds(-1); // If -1, use app's timestep.
        World world{{0.f, 0.f}};

        // Constructors
        PhysicsSystem(Scene& sceneRef);
        PhysicsSystem(PhysicsSystem&& other) = delete;
        PhysicsSystem(const PhysicsSystem& other) = delete;
        ~PhysicsSystem();

        // Operators
        PhysicsSystem& operator=(PhysicsSystem&& other) = delete;
        PhysicsSystem& operator=(const PhysicsSystem& other) = delete;

        // Functions
        void update();
    };
};