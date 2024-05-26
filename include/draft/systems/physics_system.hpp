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
        // Constructors
        PhysicsSystem(Scene& sceneRef, World& worldRef);
        ~PhysicsSystem();

        // Functions
        void update();
    };
};