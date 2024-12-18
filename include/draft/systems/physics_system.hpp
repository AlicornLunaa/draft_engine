#pragma once

#include "draft/core/application.hpp"
#include "draft/core/registry.hpp"
#include "draft/core/scene.hpp"
#include "draft/phys/world.hpp"

namespace Draft {
    class PhysicsSystem {
    private:
        // Variables
        Application* appPtr = nullptr;
        Registry& registryRef;
        World& worldRef;

        // Private functions
        void construct_body_func(Registry& reg, entt::entity rawEnt);
        void deconstruct_body_func(Registry& reg, entt::entity rawEnt);
        void construct_collider_func(Registry& reg, entt::entity rawEnt);
        void deconstruct_collider_func(Registry& reg, entt::entity rawEnt);

    public:
        // Public variables
        Time physicsTimestep = Time::seconds(-1); // If -1, use app's timestep.

        // Constructors
        PhysicsSystem(Scene& sceneRef, World& worldRef);
        PhysicsSystem(const PhysicsSystem& other) = delete;
        ~PhysicsSystem();

        // Operators
        PhysicsSystem& operator=(const PhysicsSystem& other) = delete;

        // Functions
        void update();
    };
};