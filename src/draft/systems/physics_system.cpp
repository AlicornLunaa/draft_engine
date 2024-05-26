#include "draft/core/application.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/components/rigid_body_component.hpp"
#include "draft/components/collider_component.hpp"
#include "draft/systems/physics_system.hpp"
#include "draft/phys/fixture.hpp"

namespace Draft {
    // Private functions
    void PhysicsSystem::construct_body_func(Registry& reg, entt::entity rawEnt){
        // Add colliders to new body in the simulation, first check if it has any collider components
        if(!reg.all_of<RigidBodyComponent, ColliderComponent>(rawEnt))
            // No collider or rigidbody, dont attach
            return;

        // Add the collider component to the rigidbody
        RigidBody* rigidBodyPtr = reg.get<RigidBodyComponent>(rawEnt);
        Collider& colliderRef = reg.get<ColliderComponent>(rawEnt);
        colliderRef.attach(rigidBodyPtr);
    }

    void PhysicsSystem::deconstruct_body_func(Registry& reg, entt::entity rawEnt){
        // Destroy old body in the simulation
        RigidBody& rigidBody = reg.get<RigidBodyComponent>(rawEnt);

        // Reset colliders if they were on the shape
        if(reg.all_of<ColliderComponent>(rawEnt)){
            // No colliders, nothing to update
            Collider& colliderRef = reg.get<ColliderComponent>(rawEnt);
            colliderRef.detach();
        }

        // Remove it
        rigidBody.destroy();
    }

    void PhysicsSystem::construct_collider_func(Registry& reg, entt::entity rawEnt){
        // Add colliders to new body in the simulation, first check if it has any body components
        if(!reg.all_of<RigidBodyComponent, ColliderComponent>(rawEnt))
            // No collider or rigidbody, dont attach
            return;

        // Add the collider component to the rigidbody
        RigidBody* rigidBodyPtr = reg.get<RigidBodyComponent>(rawEnt);
        Collider& colliderRef = reg.get<ColliderComponent>(rawEnt);
        colliderRef.attach(rigidBodyPtr);
    }

    void PhysicsSystem::deconstruct_collider_func(Registry& reg, entt::entity rawEnt){
        // Destroy old collider in the simulation
        if(!reg.all_of<RigidBodyComponent, ColliderComponent>(rawEnt))
            // No collider or rigidbody, dont detach
            return;

        // Add the collider component to the rigidbody
        Collider& colliderRef = reg.get<ColliderComponent>(rawEnt);
        colliderRef.detach();
    }

    // Constructors
    PhysicsSystem::PhysicsSystem(Scene& sceneRef, World& worldRef) : appPtr(sceneRef.app), registryRef(sceneRef.get_registry()), worldRef(worldRef) {
        // Attach listeners
        registryRef.on_construct<RigidBodyComponent>().connect<&PhysicsSystem::construct_body_func>(this);
        registryRef.on_construct<ColliderComponent>().connect<&PhysicsSystem::construct_collider_func>(this);
        registryRef.on_destroy<RigidBodyComponent>().connect<&PhysicsSystem::deconstruct_body_func>(this);
        registryRef.on_destroy<ColliderComponent>().connect<&PhysicsSystem::deconstruct_collider_func>(this);
    }

    PhysicsSystem::~PhysicsSystem(){
        // Remove listeners
        registryRef.on_construct<RigidBodyComponent>().disconnect<&PhysicsSystem::construct_body_func>(this);
        registryRef.on_construct<ColliderComponent>().disconnect<&PhysicsSystem::construct_collider_func>(this);
        registryRef.on_destroy<RigidBodyComponent>().disconnect<&PhysicsSystem::deconstruct_body_func>(this);
        registryRef.on_destroy<ColliderComponent>().disconnect<&PhysicsSystem::deconstruct_collider_func>(this);
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