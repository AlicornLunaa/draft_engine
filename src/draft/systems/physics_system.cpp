#include "draft/core/application.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/components/rigid_body_component.hpp"
#include "draft/components/collider_component.hpp"
#include "draft/systems/physics_system.hpp"
#include "draft/phys/fixture.hpp"

namespace Draft {
    // Private functions
    void PhysicsSystem::construct_body_func(Registry& reg, entt::entity rawEnt){
        // Get component and construct it in the world
        RigidBodyComponent& bodyComponent = reg.get<RigidBodyComponent>(rawEnt);

        // Check if it has a transform component, if it does initialize it
        if(reg.all_of<RigidBodyComponent, TransformComponent>(rawEnt)){
            TransformComponent& trans = reg.get<TransformComponent>(rawEnt);

            bodyComponent.bodyDef.position = trans.position;
            bodyComponent.bodyDef.angle = trans.rotation;

            trans.dp = trans.position;
            trans.dr = trans.rotation;
        }
        
        // Construct actual body
        bodyComponent.bodyPtr = worldRef.create_rigid_body(bodyComponent.bodyDef);

        // Add colliders to new body in the simulation, first check if it has any collider components
        if(!reg.all_of<RigidBodyComponent, ColliderComponent>(rawEnt))
            // No collider or rigidbody, dont attach
            return;

        // Add the collider component to the rigidbody
        Collider& colliderRef = reg.get<ColliderComponent>(rawEnt);
        colliderRef.attach(bodyComponent.bodyPtr);
    }

    void PhysicsSystem::deconstruct_body_func(Registry& reg, entt::entity rawEnt){
        // Destroy old body in the simulation
        RigidBodyComponent& bodyComponent = reg.get<RigidBodyComponent>(rawEnt);

        // Reset colliders if they were on the shape
        if(reg.all_of<ColliderComponent>(rawEnt)){
            // No colliders, nothing to update
            Collider& colliderRef = reg.get<ColliderComponent>(rawEnt);
            colliderRef.detach();
        }

        // Check if it has a transform component, if it does save final transform
        if(reg.all_of<RigidBodyComponent, TransformComponent>(rawEnt)){
            TransformComponent& trans = reg.get<TransformComponent>(rawEnt);
            trans.position = bodyComponent.bodyDef.position;
            trans.rotation = bodyComponent.bodyDef.angle;
        }

        // Save state to the component definition
        bodyComponent.bodyDef.position = bodyComponent.bodyPtr->get_position();
        bodyComponent.bodyDef.angle = bodyComponent.bodyPtr->get_angle();
        bodyComponent.bodyDef.linearVelocity = bodyComponent.bodyPtr->get_linear_velocity();
        bodyComponent.bodyDef.angularVelocity = bodyComponent.bodyPtr->get_angular_velocity();
        bodyComponent.bodyDef.linearDamping = bodyComponent.bodyPtr->get_linear_damping();
        bodyComponent.bodyDef.angularDamping = bodyComponent.bodyPtr->get_angular_damping();
        bodyComponent.bodyDef.awake = bodyComponent.bodyPtr->is_awake();
        bodyComponent.bodyDef.fixedRotation = bodyComponent.bodyPtr->is_fixed_rotation();
        bodyComponent.bodyDef.bullet = bodyComponent.bodyPtr->is_bullet();
        bodyComponent.bodyDef.enabled = bodyComponent.bodyPtr->is_enabled();
        bodyComponent.bodyDef.gravityScale = bodyComponent.bodyPtr->get_gravity_scale();

        // Remove it
        bodyComponent.bodyPtr->destroy();
        bodyComponent.bodyPtr = nullptr;
    }

    void PhysicsSystem::construct_collider_func(Registry& reg, entt::entity rawEnt){
        // Add colliders to new body in the simulation, first check if it has any body components
        if(!reg.all_of<RigidBodyComponent>(rawEnt))
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
            TransformComponent& trans = view.get<TransformComponent>(entity);
            RigidBody& rb = view.get<RigidBodyComponent>(entity);

            // Move based on dp and dr
            trans.dp = trans.position - trans.dp;
            trans.dr = trans.rotation - trans.dr;

            // If dp or dr is non-zero then update the body's position
            if(trans.dp.x > 0.f || trans.dp.y > 0.f || trans.dr != 0.f)
                rb.set_transform(rb.get_position() + trans.dp, rb.get_angle() + trans.dr);

            // Save new positions
            trans.position = rb.get_position();
            trans.rotation = rb.get_angle();

            // Reset deltas
            trans.dp = trans.position;
            trans.dr = trans.rotation;
        }
    }
};