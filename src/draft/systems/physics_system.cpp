#include "draft/core/application.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/components/rigid_body_component.hpp"
#include "draft/components/collider_component.hpp"
#include "draft/components/joint_component.hpp"
#include "draft/systems/physics_system.hpp"
#include "draft/phys/body_def.hpp"
#include "draft/phys/fixture.hpp"
#include "draft/phys/joint.hpp"
#include "draft/phys/joint_def.hpp"
#include "glm/common.hpp"
#include "tracy/Tracy.hpp"
#include <cassert>

namespace Draft {
    // Helper macros
    #define DRAFT_SET_IF_CHANGE(realValue, deltaValue, setFunc, getFunc) \
        if(deltaValue != realValue) setFunc; \
        realValue = getFunc; \
        deltaValue = realValue;

    // Helper functions
    template<typename ComponentType>
    void add_for_component(Entity entity){
        if(entity.has_component<ComponentType>()){
            entity.add_component<typename ComponentType::NativeType>();
        }
    }
    
    template<typename... ComponentTypes>
    void add_for_each_component(Entity entity){
        (add_for_component<ComponentTypes>(entity), ...);
    }

    template<typename ComponentType>
    void remove_for_component(Entity entity){
        if(entity.has_component<typename ComponentType::NativeType>()){
            entity.remove_component<typename ComponentType::NativeType>();
        }
    }
    
    template<typename... ComponentTypes>
    void remove_for_each_component(Entity entity){
        (remove_for_component<ComponentTypes>(entity), ...);
    }

    // Private functions
    void PhysicsSystem::construct_body_func(Registry& reg, entt::entity rawEnt){
        // A RigidBodyComponent was attached to something
        ZoneScopedN("body_construction");
        
        // Get component and construct it in the world
        RigidBodyComponent& bodyComponent = reg.get<RigidBodyComponent>(rawEnt);

        // Make sure the entity doesn't already have a native handle somehow, if it does remove it.
        if(reg.all_of<NativeBodyComponent>(rawEnt))
            reg.remove<NativeBodyComponent>(rawEnt);

        // Add a handle to the entity so it can be referenced later
        reg.emplace<NativeBodyComponent>(rawEnt);
    }

    void PhysicsSystem::construct_native_body_func(Registry& reg, entt::entity rawEnt){
        // A NativeBodyComponent was attached to something
        ZoneScopedN("body_construction");

        // Cant construct a native body without an actual body blueprint
        if(!reg.all_of<RigidBodyComponent>(rawEnt)){
            reg.remove<NativeBodyComponent>(rawEnt);
            return;
        }
        
        // Get component and construct it in the world
        NativeBodyComponent& nativeComponent = reg.get<NativeBodyComponent>(rawEnt);
        RigidBodyComponent& bodyComponent = reg.get<RigidBodyComponent>(rawEnt);
        BodyDef definition;

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

        RigidBody* body = world.create_rigid_body(definition);
        assert(body && "Something went wrong with body");

        // Add a handle to the entity so it can be referenced later
        nativeComponent.deltaP = definition.position;
        nativeComponent.deltaR = definition.angle;
        nativeComponent.deltaLinearVelocity = definition.linearVelocity;
        nativeComponent.deltaAngularVelocity = definition.angularVelocity;
        nativeComponent.bodyPtr = body;
        bodyComponent.m_nativeHandlePtr = body; // Let the bodycomponent also know about the body, albiet very strictly

        // Add colliders to the body if they exist
        if(reg.all_of<ColliderComponent>(rawEnt)){
            // Add the collider component to the rigidbody
            Collider& colliderRef = reg.get<ColliderComponent>(rawEnt);
            colliderRef.attach(body);
        }

        // Add joints to the body if they exist
        if(reg.all_of<ConstrainedComponent>(rawEnt)){
            auto& constraints = reg.get<ConstrainedComponent>(rawEnt).constraints;

            for(Entity jointEntity : constraints){
                add_for_each_component<DRAFT_ALL_JOINT_TYPES>(jointEntity);
            }
        }
    }

    void PhysicsSystem::construct_collider_func(Registry& reg, entt::entity rawEnt){
        // A collider was attached to an entity
        ZoneScopedN("collider_construction");

        // Add colliders to new body in the simulation, first check if it has any body components
        if(!reg.all_of<NativeBodyComponent>(rawEnt))
            // Cannot attach colliders without a rigidbody
            return;

        // Add the collider component to the rigidbody
        RigidBody* handle = reg.get<NativeBodyComponent>(rawEnt);
        Collider& colliderRef = reg.get<ColliderComponent>(rawEnt);
        colliderRef.attach(handle);
    }

    void PhysicsSystem::deconstruct_body_func(Registry& reg, entt::entity rawEnt){
        // BodyComponent was removed from an entity
        ZoneScopedN("body_destruction");

        // Remove the native handle from the body
        if(reg.all_of<NativeBodyComponent>(rawEnt))
            reg.remove<NativeBodyComponent>(rawEnt);
    }

    void PhysicsSystem::deconstruct_native_body_func(Registry& reg, entt::entity rawEnt){
        // NativeBodyComponent was removed from an entity
        RigidBody* body = reg.get<NativeBodyComponent>(rawEnt);

        // Skip cleanup if the body handle isnt valid
        if(!body->is_valid())
            return;

        // Check if it has a transform component, if it does save final transform
        if(reg.all_of<TransformComponent>(rawEnt)){
            TransformComponent& trans = reg.get<TransformComponent>(rawEnt);
            trans.position = body->get_position();
            trans.rotation = body->get_angle();
        }

        // Remove the collider if its on the body
        if(reg.all_of<ColliderComponent>(rawEnt)){
            // No colliders, nothing to update
            Collider& colliderRef = reg.get<ColliderComponent>(rawEnt);
            colliderRef.detach();
        }

        // Sync RigidBodyComponent data
        if(reg.all_of<RigidBodyComponent>(rawEnt)){
            RigidBodyComponent& bodyComponent = reg.get<RigidBodyComponent>(rawEnt);

            // Remove soon-to-be dangling pointer
            bodyComponent.m_nativeHandlePtr = nullptr;
        
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
        }

        // Remove joints
        if(reg.all_of<ConstrainedComponent>(rawEnt)){
            auto& constraints = reg.get<ConstrainedComponent>(rawEnt).constraints;

            for(Entity jointEntity : constraints){
                // Check for the native joint and remove it
                remove_for_each_component<DRAFT_ALL_JOINT_TYPES>(jointEntity);
            }
        }

        // Remove the physics handle
        body->destroy();
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

    void PhysicsSystem::handle_bodies(){
        // Iterate over entities and update each component, starting with transforms
        auto view1 = m_registryRef.view<TransformComponent, NativeBodyComponent>();

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
        auto view2 = m_registryRef.view<RigidBodyComponent, NativeBodyComponent>();

        for(auto entity : view2){
            RigidBodyComponent& bodyComponent = view2.get<RigidBodyComponent>(entity);
            NativeBodyComponent& handle = view2.get<NativeBodyComponent>(entity);

            if(handle.deltaType != bodyComponent.type){
                handle->set_type(bodyComponent.type);
            }
            bodyComponent.type = handle->get_type();
            handle.deltaType = bodyComponent.type;
            
            handle.deltaLinearVelocity = bodyComponent.linearVelocity - handle.deltaLinearVelocity;
            if(Math::abs(handle.deltaLinearVelocity.x) >= 0.f || Math::abs(handle.deltaLinearVelocity.y) >= 0.f)
                handle->set_linear_velocity(handle->get_linear_velocity() + handle.deltaLinearVelocity);
            bodyComponent.linearVelocity = handle->get_linear_velocity();
            handle.deltaLinearVelocity = bodyComponent.linearVelocity;

            handle.deltaAngularVelocity = bodyComponent.angularVelocity - handle.deltaAngularVelocity;
            if(Math::abs(handle.deltaAngularVelocity) >= 0.f)
                handle->set_angular_velocity(handle->get_angular_velocity() + handle.deltaAngularVelocity);
            bodyComponent.angularVelocity = handle->get_angular_velocity();
            handle.deltaAngularVelocity = bodyComponent.angularVelocity;

            handle.deltaLinearDamping = bodyComponent.linearDamping - handle.deltaLinearDamping;
            if(Math::abs(handle.deltaLinearDamping) >= 0.f)
                handle->set_linear_damping(handle->get_linear_damping() + handle.deltaLinearDamping);
            bodyComponent.linearDamping = handle->get_linear_damping();
            handle.deltaLinearDamping = bodyComponent.linearDamping;

            handle.deltaAngularDamping = bodyComponent.angularDamping - handle.deltaAngularDamping;
            if(Math::abs(handle.deltaAngularDamping) >= 0.f)
                handle->set_angular_damping(handle->get_angular_damping() + handle.deltaAngularDamping);
            bodyComponent.angularDamping = handle->get_angular_damping();
            handle.deltaAngularDamping = bodyComponent.angularDamping;

            if(handle.deltaAwake != bodyComponent.awake){
                if(bodyComponent.awake){
                    handle->set_awake();
                } else {
                    handle->set_sleep();
                }
            }
            bodyComponent.awake = handle->is_awake();
            handle.deltaAwake = bodyComponent.awake;

            if(handle.deltaFixedRotation != bodyComponent.fixedRotation)
                handle->set_fixed_rotation(bodyComponent.fixedRotation);
            bodyComponent.fixedRotation = handle->is_fixed_rotation();
            handle.deltaFixedRotation = bodyComponent.fixedRotation;

            if(handle.deltaBullet != bodyComponent.bullet)
                handle->set_bullet(bodyComponent.bullet);
            bodyComponent.bullet = handle->is_bullet();
            handle.deltaBullet = bodyComponent.bullet;

            if(handle.deltaEnabled != bodyComponent.enabled)
                handle->set_enabled(bodyComponent.enabled);
            bodyComponent.enabled = handle->is_enabled();
            handle.deltaEnabled = bodyComponent.enabled;

            handle.deltaGravityScale = bodyComponent.gravityScale - handle.deltaGravityScale;
            if(Math::abs(handle.deltaGravityScale) >= 0.f)
                handle->set_gravity_scale(handle->get_gravity_scale() + handle.deltaGravityScale);
            bodyComponent.gravityScale = handle->get_gravity_scale();
            handle.deltaGravityScale = bodyComponent.gravityScale;
        }
    }

    void PhysicsSystem::handle_forces(){
        // Handle forces
        // Torques
        auto view3 = m_registryRef.view<TorqueComponent, NativeBodyComponent>();

        for(auto entity : view3){
            TorqueComponent& physComponent = view3.get<TorqueComponent>(entity);
            RigidBody* body = view3.get<NativeBodyComponent>(entity);
            body->apply_torque(physComponent.torque, physComponent.wake);
            m_registryRef.remove<TorqueComponent>(entity);
        }

        // Forces
        auto view4 = m_registryRef.view<ForceComponent, NativeBodyComponent>();

        for(auto entity : view4){
            ForceComponent& physComponent = view4.get<ForceComponent>(entity);
            RigidBody* body = view4.get<NativeBodyComponent>(entity);
            body->apply_force(physComponent.force, physComponent.point, physComponent.wake);
            m_registryRef.remove<ForceComponent>(entity);
        }

        // Impulses
        auto view5 = m_registryRef.view<ImpulseComponent, NativeBodyComponent>();

        for(auto entity : view5){
            ImpulseComponent& physComponent = view5.get<ImpulseComponent>(entity);
            RigidBody* body = view5.get<NativeBodyComponent>(entity);
            body->apply_linear_impulse(physComponent.force, physComponent.point, physComponent.wake);
            body->apply_angular_impulse(physComponent.angular, physComponent.wake);
            m_registryRef.remove<ImpulseComponent>(entity);
        }

        // Continuous torque
        auto view6 = m_registryRef.view<ContinuousTorqueComponent, NativeBodyComponent>();

        for(auto entity : view6){
            ContinuousTorqueComponent& physComponent = view6.get<ContinuousTorqueComponent>(entity);
            RigidBody* body = view6.get<NativeBodyComponent>(entity);

            if(physComponent.enabled)
                body->apply_torque(physComponent.torque, physComponent.wake);
        }

        // Continuous force
        auto view7 = m_registryRef.view<ContinuousForceComponent, NativeBodyComponent>();

        for(auto entity : view7){
            ContinuousForceComponent& physComponent = view7.get<ContinuousForceComponent>(entity);
            RigidBody* body = view7.get<NativeBodyComponent>(entity);

            if(physComponent.enabled)
                body->apply_force(physComponent.force, physComponent.point, physComponent.wake);
        }

        // Continuous impulse
        auto view8 = m_registryRef.view<ContinuousImpulseComponent, NativeBodyComponent>();

        for(auto entity : view8){
            ContinuousImpulseComponent& physComponent = view8.get<ContinuousImpulseComponent>(entity);
            RigidBody* body = view8.get<NativeBodyComponent>(entity);

            if(physComponent.enabled){
                body->apply_linear_impulse(physComponent.force, physComponent.point, physComponent.wake);
                body->apply_angular_impulse(physComponent.angular, physComponent.wake);
            }
        }
    }

    void PhysicsSystem::handle_joints(){
        // Loops through every kind of joint and runs logic to keep the native and data components syncronized
        {
            // Distance joints
            auto view = m_registryRef.view<DistanceJointComponent, DistanceJointComponent::NativeType>();

            for(auto entity : view){
                auto& jointData = view.get<DistanceJointComponent>(entity);
                auto& handle = view.get<DistanceJointComponent::NativeType>(entity);
                auto* ptr = handle.get_as<DistanceJoint>();
                
                if(handle.delta.anchorA != jointData.anchorA)
                    jointData.anchorA = handle.delta.anchorA;

                if(handle.delta.anchorB != jointData.anchorB)
                    jointData.anchorB = handle.delta.anchorB;

                DRAFT_SET_IF_CHANGE(jointData.length, handle.delta.length, ptr->set_length(jointData.length), ptr->get_length());
                DRAFT_SET_IF_CHANGE(jointData.minLength, handle.delta.minLength, ptr->set_min_length(jointData.minLength), ptr->get_min_length());
                DRAFT_SET_IF_CHANGE(jointData.maxLength, handle.delta.maxLength, ptr->set_max_length(jointData.maxLength), ptr->get_max_length());
                DRAFT_SET_IF_CHANGE(jointData.stiffness, handle.delta.stiffness, ptr->set_stiffness(jointData.stiffness), ptr->get_stiffness());
                DRAFT_SET_IF_CHANGE(jointData.damping, handle.delta.damping, ptr->set_damping(jointData.damping), ptr->get_damping());
            }
        }
        {
            // Friction joints
            auto view = m_registryRef.view<FrictionJointComponent, FrictionJointComponent::NativeType>();

            for(auto entity : view){
                auto& jointData = view.get<FrictionJointComponent>(entity);
                auto& handle = view.get<FrictionJointComponent::NativeType>(entity);
                auto* ptr = handle.get_as<FrictionJoint>();
                
                if(handle.delta.anchorA != jointData.anchorA)
                    jointData.anchorA = handle.delta.anchorA;

                if(handle.delta.anchorB != jointData.anchorB)
                    jointData.anchorB = handle.delta.anchorB;

                DRAFT_SET_IF_CHANGE(jointData.maxForce, handle.delta.maxForce, ptr->set_max_force(jointData.maxForce), ptr->get_max_force());
                DRAFT_SET_IF_CHANGE(jointData.maxTorque, handle.delta.maxTorque, ptr->set_max_torque(jointData.maxTorque), ptr->get_max_torque());
            }
        }
        {
            // Gear joints
            auto view = m_registryRef.view<GearJointComponent, GearJointComponent::NativeType>();

            for(auto entity : view){
                auto& jointData = view.get<GearJointComponent>(entity);
                auto& handle = view.get<GearJointComponent::NativeType>(entity);
                auto* ptr = handle.get_as<GearJoint>();
                DRAFT_SET_IF_CHANGE(jointData.ratio, handle.delta.ratio, ptr->set_ratio(jointData.ratio), ptr->get_ratio());
            }
        }
        {
            // Motor joints
            auto view = m_registryRef.view<MotorJointComponent, MotorJointComponent::NativeType>();

            for(auto entity : view){
                auto& jointData = view.get<MotorJointComponent>(entity);
                auto& handle = view.get<MotorJointComponent::NativeType>(entity);
                auto* ptr = handle.get_as<MotorJoint>();

                DRAFT_SET_IF_CHANGE(jointData.linearOffset, handle.delta.linearOffset, ptr->set_linear_offset(jointData.linearOffset), ptr->get_linear_offset());
                DRAFT_SET_IF_CHANGE(jointData.angularOffset, handle.delta.angularOffset, ptr->set_angular_offset(jointData.angularOffset), ptr->get_angular_offset());
                DRAFT_SET_IF_CHANGE(jointData.maxForce, handle.delta.maxForce, ptr->set_max_force(jointData.maxForce), ptr->get_max_force());
                DRAFT_SET_IF_CHANGE(jointData.maxTorque, handle.delta.maxTorque, ptr->set_max_torque(jointData.maxTorque), ptr->get_max_torque());
                DRAFT_SET_IF_CHANGE(jointData.correctionFactor, handle.delta.correctionFactor, ptr->set_correction_factor(jointData.correctionFactor), ptr->get_correction_factor());
            }
        }
        {
            // Mouse joints
            auto view = m_registryRef.view<MouseJointComponent, MouseJointComponent::NativeType>();

            for(auto entity : view){
                auto& jointData = view.get<MouseJointComponent>(entity);
                auto& handle = view.get<MouseJointComponent::NativeType>(entity);
                auto* ptr = handle.get_as<MouseJoint>();

                DRAFT_SET_IF_CHANGE(jointData.target, handle.delta.target, ptr->set_target(jointData.target), ptr->get_target());
                DRAFT_SET_IF_CHANGE(jointData.maxForce, handle.delta.maxForce, ptr->set_max_force(jointData.maxForce), ptr->get_max_force());
                DRAFT_SET_IF_CHANGE(jointData.stiffness, handle.delta.stiffness, ptr->set_stiffness(jointData.stiffness), ptr->get_stiffness());
                DRAFT_SET_IF_CHANGE(jointData.damping, handle.delta.damping, ptr->set_damping(jointData.damping), ptr->get_damping());
            }
        }
        {
            // Prismatic joints
            auto view = m_registryRef.view<PrismaticJointComponent, PrismaticJointComponent::NativeType>();

            for(auto entity : view){
                auto& jointData = view.get<PrismaticJointComponent>(entity);
                auto& handle = view.get<PrismaticJointComponent::NativeType>(entity);
                auto* ptr = handle.get_as<PrismaticJoint>();
                
                if(handle.delta.anchorA != jointData.anchorA)
                    jointData.anchorA = handle.delta.anchorA;

                if(handle.delta.anchorB != jointData.anchorB)
                    jointData.anchorB = handle.delta.anchorB;

                if(handle.delta.localAxisA != jointData.localAxisA)
                    jointData.localAxisA = handle.delta.localAxisA;

                if(handle.delta.referenceAngle != jointData.referenceAngle)
                    jointData.referenceAngle = handle.delta.referenceAngle;

                DRAFT_SET_IF_CHANGE(jointData.lowerTranslation, handle.delta.lowerTranslation, ptr->set_limits(jointData.lowerTranslation, ptr->get_upper_limit()), ptr->get_lower_limit());
                DRAFT_SET_IF_CHANGE(jointData.upperTranslation, handle.delta.upperTranslation, ptr->set_limits(ptr->get_lower_limit(), jointData.upperTranslation), ptr->get_upper_limit());
                DRAFT_SET_IF_CHANGE(jointData.maxMotorForce, handle.delta.maxMotorForce, ptr->set_max_motor_force(jointData.maxMotorForce), ptr->get_max_motor_force());
                DRAFT_SET_IF_CHANGE(jointData.motorSpeed, handle.delta.motorSpeed, ptr->set_motor_speed(jointData.motorSpeed), ptr->get_motor_speed());
                DRAFT_SET_IF_CHANGE(jointData.enableLimit, handle.delta.enableLimit, ptr->enable_limit(jointData.enableLimit), ptr->is_limit_enabled());
                DRAFT_SET_IF_CHANGE(jointData.enableMotor, handle.delta.enableMotor, ptr->enable_motor(jointData.enableMotor), ptr->is_motor_enabled());
            }
        }
        {
            // Pulley joints
            auto view = m_registryRef.view<PulleyJointComponent, PulleyJointComponent::NativeType>();

            for(auto entity : view){
                auto& jointData = view.get<PulleyJointComponent>(entity);
                auto& handle = view.get<PulleyJointComponent::NativeType>(entity);
                auto* ptr = handle.get_as<PulleyJoint>();
                
                if(handle.delta.groundAnchorA != jointData.groundAnchorA)
                    jointData.groundAnchorA = handle.delta.groundAnchorA;

                if(handle.delta.groundAnchorB != jointData.groundAnchorB)
                    jointData.groundAnchorB = handle.delta.groundAnchorB;
                
                if(handle.delta.localAnchorA != jointData.localAnchorA)
                    jointData.localAnchorA = handle.delta.localAnchorA;

                if(handle.delta.localAnchorB != jointData.localAnchorB)
                    jointData.localAnchorB = handle.delta.localAnchorB;

                jointData.lengthA = ptr->get_length_a();
                jointData.lengthB = ptr->get_length_b();
                jointData.ratio = ptr->get_ratio();
            }
        }
        {
            // Revolute joints
            auto view = m_registryRef.view<RevoluteJointComponent, RevoluteJointComponent::NativeType>();

            for(auto entity : view){
                auto& jointData = view.get<RevoluteJointComponent>(entity);
                auto& handle = view.get<RevoluteJointComponent::NativeType>(entity);
                auto* ptr = handle.get_as<RevoluteJoint>();
                
                if(handle.delta.localAnchorA != jointData.localAnchorA)
                    jointData.localAnchorA = handle.delta.localAnchorA;

                if(handle.delta.localAnchorB != jointData.localAnchorB)
                    jointData.localAnchorB = handle.delta.localAnchorB;

                if(handle.delta.referenceAngle != jointData.referenceAngle)
                    jointData.referenceAngle = handle.delta.referenceAngle;

                DRAFT_SET_IF_CHANGE(jointData.lowerAngle, handle.delta.lowerAngle, ptr->set_limits(jointData.lowerAngle, ptr->get_upper_limit()), ptr->get_lower_limit());
                DRAFT_SET_IF_CHANGE(jointData.upperAngle, handle.delta.upperAngle, ptr->set_limits(ptr->get_lower_limit(), jointData.upperAngle), ptr->get_upper_limit());
                DRAFT_SET_IF_CHANGE(jointData.maxMotorTorque, handle.delta.maxMotorTorque, ptr->set_max_motor_torque(jointData.maxMotorTorque), ptr->get_max_motor_torque());
                DRAFT_SET_IF_CHANGE(jointData.motorSpeed, handle.delta.motorSpeed, ptr->set_motor_speed(jointData.motorSpeed), ptr->get_motor_speed());
                DRAFT_SET_IF_CHANGE(jointData.enableLimit, handle.delta.enableLimit, ptr->enable_limit(jointData.enableLimit), ptr->is_limit_enabled());
                DRAFT_SET_IF_CHANGE(jointData.enableMotor, handle.delta.enableMotor, ptr->enable_motor(jointData.enableMotor), ptr->is_motor_enabled());
            }
        }
        {
            // Weld joints
            auto view = m_registryRef.view<WeldJointComponent, WeldJointComponent::NativeType>();

            for(auto entity : view){
                auto& jointData = view.get<WeldJointComponent>(entity);
                auto& handle = view.get<WeldJointComponent::NativeType>(entity);
                auto* ptr = handle.get_as<WeldJoint>();
                
                if(handle.delta.anchorA != jointData.anchorA)
                    jointData.anchorA = handle.delta.anchorA;

                if(handle.delta.anchorB != jointData.anchorB)
                    jointData.anchorB = handle.delta.anchorB;

                if(handle.delta.referenceAngle != jointData.referenceAngle)
                    jointData.referenceAngle = handle.delta.referenceAngle;

                DRAFT_SET_IF_CHANGE(jointData.stiffness, handle.delta.stiffness, ptr->set_stiffness(jointData.stiffness), ptr->get_stiffness());
                DRAFT_SET_IF_CHANGE(jointData.damping, handle.delta.damping, ptr->set_damping(jointData.damping), ptr->get_damping());
            }
        }
        {
            // Wheel joints
            auto view = m_registryRef.view<WheelJointComponent, WheelJointComponent::NativeType>();

            for(auto entity : view){
                auto& jointData = view.get<WheelJointComponent>(entity);
                auto& handle = view.get<WheelJointComponent::NativeType>(entity);
                auto* ptr = handle.get_as<WheelJoint>();
                
                if(handle.delta.anchorA != jointData.anchorA)
                    jointData.anchorA = handle.delta.anchorA;

                if(handle.delta.anchorB != jointData.anchorB)
                    jointData.anchorB = handle.delta.anchorB;

                if(handle.delta.localAxis != jointData.localAxis)
                    jointData.localAxis = handle.delta.localAxis;

                DRAFT_SET_IF_CHANGE(jointData.lowerTranslation, handle.delta.lowerTranslation, ptr->set_limits(jointData.lowerTranslation, ptr->get_upper_limit()), ptr->get_lower_limit());
                DRAFT_SET_IF_CHANGE(jointData.upperTranslation, handle.delta.upperTranslation, ptr->set_limits(ptr->get_lower_limit(), jointData.upperTranslation), ptr->get_upper_limit());
                DRAFT_SET_IF_CHANGE(jointData.maxMotorTorque, handle.delta.maxMotorTorque, ptr->set_max_motor_torque(jointData.maxMotorTorque), ptr->get_max_motor_torque());
                DRAFT_SET_IF_CHANGE(jointData.motorSpeed, handle.delta.motorSpeed, ptr->set_motor_speed(jointData.motorSpeed), ptr->get_motor_speed());
                DRAFT_SET_IF_CHANGE(jointData.stiffness, handle.delta.stiffness, ptr->set_stiffness(jointData.stiffness), ptr->get_stiffness());
                DRAFT_SET_IF_CHANGE(jointData.damping, handle.delta.damping, ptr->set_damping(jointData.damping), ptr->get_damping());
                DRAFT_SET_IF_CHANGE(jointData.enableLimit, handle.delta.enableLimit, ptr->enable_limit(jointData.enableLimit), ptr->is_limit_enabled());
                DRAFT_SET_IF_CHANGE(jointData.enableMotor, handle.delta.enableMotor, ptr->enable_motor(jointData.enableMotor), ptr->is_motor_enabled());
            }
        }
    }

    // Constructors
    PhysicsSystem::PhysicsSystem(Scene& sceneRef) : m_appPtr(sceneRef.get_app()), m_registryRef(sceneRef.get_registry()), m_sceneRef(sceneRef) {
        // Attach listeners
        m_registryRef.on_construct<RigidBodyComponent>().connect<&PhysicsSystem::construct_body_func>(this);
        m_registryRef.on_construct<NativeBodyComponent>().connect<&PhysicsSystem::construct_native_body_func>(this);
        m_registryRef.on_construct<ColliderComponent>().connect<&PhysicsSystem::construct_collider_func>(this);

        m_registryRef.on_destroy<RigidBodyComponent>().connect<&PhysicsSystem::deconstruct_body_func>(this);
        m_registryRef.on_destroy<NativeBodyComponent>().connect<&PhysicsSystem::deconstruct_native_body_func>(this);
        m_registryRef.on_destroy<ColliderComponent>().connect<&PhysicsSystem::deconstruct_collider_func>(this);

        attach_listeners_for_all<DRAFT_ALL_JOINT_TYPES>(m_registryRef, this);
    }

    PhysicsSystem::~PhysicsSystem(){
        // Remove listeners
        m_registryRef.on_construct<RigidBodyComponent>().disconnect<&PhysicsSystem::construct_body_func>(this);
        m_registryRef.on_construct<NativeBodyComponent>().disconnect<&PhysicsSystem::construct_native_body_func>(this);
        m_registryRef.on_construct<ColliderComponent>().disconnect<&PhysicsSystem::construct_collider_func>(this);
        
        m_registryRef.on_destroy<RigidBodyComponent>().disconnect<&PhysicsSystem::deconstruct_body_func>(this);
        m_registryRef.on_destroy<NativeBodyComponent>().disconnect<&PhysicsSystem::deconstruct_native_body_func>(this);
        m_registryRef.on_destroy<ColliderComponent>().disconnect<&PhysicsSystem::deconstruct_collider_func>(this);

        detach_listeners_for_all<DRAFT_ALL_JOINT_TYPES>(m_registryRef, this);
    }

    // Functions
    void PhysicsSystem::update(){
        // Update physics world
        ZoneScopedN("physics_system");

        Time prevTimestep = m_appPtr->timeStep;
        bool resetTime = false;

        if(physicsTimestep.as_seconds() >= 0.f){
            m_appPtr->timeStep = physicsTimestep;
            resetTime = true;
        }

        world.step(m_appPtr->timeStep, world.VELOCITY_ITER, world.POSITION_ITER);

        // Views
        handle_joints();
        handle_forces();
        handle_bodies();
        
        // Reset accumulator
        if(resetTime){
            m_appPtr->timeStep = prevTimestep;
        }
    }
};