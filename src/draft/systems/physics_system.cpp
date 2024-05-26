#include "draft/core/application.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/components/rigid_body_component.hpp"
#include "draft/systems/physics_system.hpp"
#include "draft/phys/fixture.hpp"

#include <iostream>

namespace Draft {
    // Private functions
    void PhysicsSystem::construct_func(Registry& reg, entt::entity rawEnt){
        std::cout << "Hello " << static_cast<uint32_t>(rawEnt) << "\n";
    }

    void PhysicsSystem::deconstruct_func(Registry& reg, entt::entity rawEnt){
        std::cout << "Goodbye " << static_cast<uint32_t>(rawEnt) << "\n";

        auto& rigidBodyComponent = reg.get<RigidBodyComponent>(rawEnt);
    }

    // Constructors
    PhysicsSystem::PhysicsSystem(Scene& sceneRef, World& worldRef) : appPtr(sceneRef.app), registryRef(sceneRef.get_registry()), worldRef(worldRef) {
        // Attach listeners
        registryRef.on_construct<RigidBodyComponent>().connect<&PhysicsSystem::construct_func>(this);
        registryRef.on_destroy<RigidBodyComponent>().connect<&PhysicsSystem::deconstruct_func>(this);
    }

    PhysicsSystem::~PhysicsSystem(){
        // Remove listeners
        registryRef.on_destroy<RigidBodyComponent>().disconnect<&PhysicsSystem::deconstruct_func>(this);
        registryRef.on_construct<RigidBodyComponent>().disconnect<&PhysicsSystem::construct_func>(this);
    }

    // Functions
    void PhysicsSystem::update(){
        worldRef.step(appPtr->timeStep, worldRef.VELOCITY_ITER, worldRef.POSITION_ITER);

        auto view = registryRef.view<TransformComponent, RigidBodyComponent>();

        for(auto entity : view){
            TransformComponent& transformComponent = view.get<TransformComponent>(entity);
            RigidBody& rigidBodyComponent = view.get<RigidBodyComponent>(entity);

            transformComponent.position = rigidBodyComponent.get_position();
            transformComponent.rotation = rigidBodyComponent.get_angle();
        }
    }
};