#include "draft/core/application.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/components/rigid_body_component.hpp"
#include "draft/components/collider_component.hpp"
#include "draft/systems/physics_system.hpp"
#include "draft/phys/body_def.hpp"
#include "draft/phys/fixture.hpp"
#include "tracy/Tracy.hpp"
#include <cassert>

namespace Draft {
    // Private functions
    void PhysicsSystem::construct_body_func(Registry& reg, entt::entity rawEnt){
        // A RigidBodyComponent was attached to something
        ZoneScopedN("body_construction");
        
        // Get component and construct it in the world
        RigidBodyComponent& bodyComponent = reg.get<RigidBodyComponent>(rawEnt);
        BodyDef definition;

        // Make sure the entity doesn't already have a native handle somehow
        if(reg.all_of<NativeBodyComponent>(rawEnt))
            return;

        // A rigidbody can only be constructed if there's a transform, otherwise the body can't move
        if(reg.all_of<TransformComponent>(rawEnt)){
            TransformComponent& trans = reg.get<TransformComponent>(rawEnt);
            definition.position = trans.position;
            definition.angle = trans.rotation;
        }
        
        // Construct actual body
        definition.type = bodyComponent.type;
        definition.linearVelocity = bodyComponent.linearVelocity;
        definition.angularVelocity = bodyComponent.angularVelocity;
        definition.linearDamping = bodyComponent.linearDamping;
        definition.angularDamping = bodyComponent.angularDamping;
        definition.allowSleep = bodyComponent.allowSleep;
        definition.awake = bodyComponent.awake;
        definition.fixedRotation = bodyComponent.fixedRotation;
        definition.bullet = bodyComponent.bullet;
        definition.enabled = bodyComponent.enabled;
        definition.gravityScale = bodyComponent.gravityScale;

        RigidBody* body = worldRef.create_rigid_body(definition);
        assert(body && "Something went wrong with body");

        // Add a handle to the entity so it can be referenced later
        reg.emplace<NativeBodyComponent>(rawEnt, body, body->get_position(), body->get_angle());

        // Add colliders to the body if they exist
        if(reg.all_of<ColliderComponent>(rawEnt)){
            // Add the collider component to the rigidbody
            Collider& colliderRef = reg.get<ColliderComponent>(rawEnt);
            colliderRef.attach(body);
        }
    }

    void PhysicsSystem::construct_collider_func(Registry& reg, entt::entity rawEnt){
        // A collider was attached to an entity
        ZoneScopedN("collider_construction");

        // Add colliders to new body in the simulation, first check if it has any body components
        if(!reg.all_of<NativeBodyComponent>(rawEnt))
            // No collider or rigidbody, dont attach
            return;

        // Add the collider component to the rigidbody
        RigidBody* rigidBodyPtr = reg.get<NativeBodyComponent>(rawEnt);
        Collider& colliderRef = reg.get<ColliderComponent>(rawEnt);
        colliderRef.attach(rigidBodyPtr);
    }

    void PhysicsSystem::deconstruct_body_func(Registry& reg, entt::entity rawEnt){
        // Body was removed from an entity
        ZoneScopedN("body_destruction");

        // Make sure the body is actually physically active
        if(!reg.all_of<NativeBodyComponent>(rawEnt))
            return;

        // Destroy old body in the simulation
        RigidBodyComponent& bodyComponent = reg.get<RigidBodyComponent>(rawEnt);
        RigidBody* body = reg.get<NativeBodyComponent>(rawEnt);

        // Remove the collider if its on the body
        if(reg.all_of<ColliderComponent>(rawEnt)){
            // No colliders, nothing to update
            Collider& colliderRef = reg.get<ColliderComponent>(rawEnt);
            colliderRef.detach();
        }

        // Check if it has a transform component, if it does save final transform
        if(reg.all_of<TransformComponent>(rawEnt)){
            TransformComponent& trans = reg.get<TransformComponent>(rawEnt);
            trans.position = body->get_position();
            trans.rotation = body->get_angle();
        }

        // Save state to the component definition
        bodyComponent.linearVelocity = body->get_linear_velocity();
        bodyComponent.angularVelocity = body->get_angular_velocity();
        bodyComponent.linearDamping = body->get_linear_damping();
        bodyComponent.angularDamping = body->get_angular_damping();
        bodyComponent.awake = body->is_awake();
        bodyComponent.fixedRotation = body->is_fixed_rotation();
        bodyComponent.bullet = body->is_bullet();
        bodyComponent.enabled = body->is_enabled();
        bodyComponent.gravityScale = body->get_gravity_scale();

        // Remove it
        body->destroy();
        reg.remove<NativeBodyComponent>(rawEnt);
    }

    void PhysicsSystem::deconstruct_collider_func(Registry& reg, entt::entity rawEnt){
        // Collider was removed from an entity object
        ZoneScopedN("collider_destruction");

        // Check to make sure there was a body before removing
        if(!reg.all_of<NativeBodyComponent>(rawEnt))
            return;

        // Detach collider
        Collider& colliderRef = reg.get<ColliderComponent>(rawEnt);
        colliderRef.detach();
    }

    // Constructors
    PhysicsSystem::PhysicsSystem(Scene& sceneRef, World& worldRef) : appPtr(sceneRef.get_app()), registryRef(sceneRef.get_registry()), worldRef(worldRef) {
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
        // Update physics world
        ZoneScopedN("physics_system");

        Time prevTimestep = appPtr->timeStep;
        bool resetTime = false;

        if(physicsTimestep.as_seconds() >= 0.f){
            appPtr->timeStep = physicsTimestep;
            resetTime = true;
        }

        worldRef.step(appPtr->timeStep, worldRef.VELOCITY_ITER, worldRef.POSITION_ITER);

        // Iterate over entities and update each component, starting with transforms
        auto view1 = registryRef.view<TransformComponent, NativeBodyComponent>();

        for(auto entity : view1){
            TransformComponent& transform = view1.get<TransformComponent>(entity);
            NativeBodyComponent& nativeHandle = view1.get<NativeBodyComponent>(entity);
            RigidBody& body = nativeHandle;

            // Move based on dp and dr
            nativeHandle.deltaP = transform.position - nativeHandle.deltaP;
            nativeHandle.deltaR = transform.rotation - nativeHandle.deltaR;

            // If dp or dr is non-zero then update the body's position
            if(Math::abs(nativeHandle.deltaP.x) > 0.f || Math::abs(nativeHandle.deltaP.y) > 0.f || nativeHandle.deltaR != 0.f)
                body.set_transform(body.get_position() + nativeHandle.deltaP, body.get_angle() + nativeHandle.deltaR);

            // Save new positions
            transform.position = body.get_position();
            transform.rotation = body.get_angle();

            // Reset deltas
            nativeHandle.deltaP = transform.position;
            nativeHandle.deltaR = transform.rotation;
        }

        // Sync rigidbody state
        auto view2 = registryRef.view<RigidBodyComponent, NativeBodyComponent>();

        for(auto entity : view2){
            RigidBodyComponent& bodyComponent = view2.get<RigidBodyComponent>(entity);
            RigidBody* body = view2.get<NativeBodyComponent>(entity);
            
            bodyComponent.linearVelocity = body->get_linear_velocity();
            bodyComponent.angularVelocity = body->get_angular_velocity();
            bodyComponent.linearDamping = body->get_linear_damping();
            bodyComponent.angularDamping = body->get_angular_damping();
            bodyComponent.awake = body->is_awake();
            bodyComponent.fixedRotation = body->is_fixed_rotation();
            bodyComponent.bullet = body->is_bullet();
            bodyComponent.enabled = body->is_enabled();
            bodyComponent.gravityScale = body->get_gravity_scale();
        }

        // Handle forces
        // Torques
        auto view3 = registryRef.view<TorqueComponent, NativeBodyComponent>();

        for(auto entity : view3){
            TorqueComponent& physComponent = view3.get<TorqueComponent>(entity);
            RigidBody* body = view3.get<NativeBodyComponent>(entity);
            body->apply_torque(physComponent.torque, physComponent.wake);
            registryRef.remove<TorqueComponent>(entity);
        }

        // Forces
        auto view4 = registryRef.view<ForceComponent, NativeBodyComponent>();

        for(auto entity : view4){
            ForceComponent& physComponent = view4.get<ForceComponent>(entity);
            RigidBody* body = view4.get<NativeBodyComponent>(entity);
            body->apply_force(physComponent.force, physComponent.point, physComponent.wake);
            registryRef.remove<ForceComponent>(entity);
        }

        // Impulses
        auto view5 = registryRef.view<ImpulseComponent, NativeBodyComponent>();

        for(auto entity : view5){
            ImpulseComponent& physComponent = view5.get<ImpulseComponent>(entity);
            RigidBody* body = view5.get<NativeBodyComponent>(entity);
            body->apply_linear_impulse(physComponent.force, physComponent.point, physComponent.wake);
            body->apply_angular_impulse(physComponent.angular, physComponent.wake);
            registryRef.remove<ImpulseComponent>(entity);
        }

        // Continuous torque
        auto view6 = registryRef.view<ContinuousTorqueComponent, NativeBodyComponent>();

        for(auto entity : view6){
            ContinuousTorqueComponent& physComponent = view6.get<ContinuousTorqueComponent>(entity);
            RigidBody* body = view6.get<NativeBodyComponent>(entity);

            if(physComponent.enabled)
                body->apply_torque(physComponent.torque, physComponent.wake);
        }

        // Continuous force
        auto view7 = registryRef.view<ContinuousForceComponent, NativeBodyComponent>();

        for(auto entity : view7){
            ContinuousForceComponent& physComponent = view7.get<ContinuousForceComponent>(entity);
            RigidBody* body = view7.get<NativeBodyComponent>(entity);

            if(physComponent.enabled)
                body->apply_force(physComponent.force, physComponent.point, physComponent.wake);
        }

        // Continuous impulse
        auto view8 = registryRef.view<ContinuousImpulseComponent, NativeBodyComponent>();

        for(auto entity : view8){
            ContinuousImpulseComponent& physComponent = view8.get<ContinuousImpulseComponent>(entity);
            RigidBody* body = view8.get<NativeBodyComponent>(entity);

            if(physComponent.enabled){
                body->apply_linear_impulse(physComponent.force, physComponent.point, physComponent.wake);
                body->apply_angular_impulse(physComponent.angular, physComponent.wake);
            }
        }
        
        // Reset accumulator
        if(resetTime){
            appPtr->timeStep = prevTimestep;
        }
    }
};