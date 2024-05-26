#include "draft/core/application.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/components/rigid_body_component.hpp"
#include "draft/systems/physics_system.hpp"
#include "draft/phys/fixture.hpp"

namespace Draft {
    // Constructors
    PhysicsSystem::PhysicsSystem(Scene& sceneRef, World& worldRef) : appPtr(sceneRef.app), registryRef(sceneRef.get_registry()), worldRef(worldRef) {
        // Attach listeners

    }

    PhysicsSystem::~PhysicsSystem(){
        // Remove listeners

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