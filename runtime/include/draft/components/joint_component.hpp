#pragma once

#include "draft/ecs/entity.hpp"
#include "draft/ecs/scene.hpp"
#include "draft/physics/joint.hpp"
#include "draft/physics/joint_data.hpp"

#include <tuple>
#include <type_traits>
#include <vector>

#define DRAFT_MAYBE_OR_ELSE(ptr, returnValue, failureValue) if(ptr) return ptr->returnValue; else return failureValue;
#define DRAFT_MAYBE(ptr, returnValue) if(ptr) return ptr->returnValue;

namespace Draft {
    // Base components
    /**
     * @brief Live `Joint*` handle + the delta-tracking `T` blueprint, added/removed alongside a
     * `JointComponent<T>` by `PhysicsSystem`. Not `DRAFT_REFLECTABLE`, `jointPtr` is a raw
     * pointer to a live handle, same reasoning as `FixtureDef::shape`.
     */
    template<typename T>
    struct NativeJointComponent {
        // Protections
        static_assert(std::is_base_of<GenericJointData, T>::value, "T must be derived from GenericJointData");

        // Handles
        Joint* jointPtr = nullptr;
        T delta;

        // Helpers
        Joint* operator->(){ return jointPtr; }
        operator Joint& () { return *jointPtr; }
        operator Joint* () { return jointPtr; }
        operator const Joint* () const { return jointPtr; }
        operator bool () const { return is_valid(); }

        bool is_valid() const { return jointPtr != nullptr; }

        template<typename K>
        K* get_as() {
            static_assert(std::is_base_of<Joint, K>::value, "Cannot obtain a joint as anything other than a descendant of joint");
            return static_cast<K*>(jointPtr);
        }
    };

    /**
     * @brief Abstracted joint blueprint/state between `entityA` and `entityB`. `PhysicsSystem`
     * keeps this synced with the live `NativeJointComponent<T>` every physics step.
     */
    template<typename T>
    struct JointComponent : public T {
        // Protections
        static_assert(std::is_base_of<GenericJointData, T>::value, "T must be derived from GenericJointData");
        using JointDataType = T;
        using NativeType = NativeJointComponent<JointDataType>;

        // Joint state and blueprint
        Entity entityA;
        Entity entityB;

        // Native handle for the indirection since Joint is handled with NativeJointComponent
        Joint* m_nativeHandlePtr = nullptr;

        // Functions
        inline Joint::Type get_type() const { DRAFT_MAYBE_OR_ELSE(m_nativeHandlePtr, get_type(), Joint::Type::UNKNOWN); }
        inline Vector2f get_world_anchor_a() const { DRAFT_MAYBE_OR_ELSE(m_nativeHandlePtr, get_world_anchor_a(), {}); }
        inline Vector2f get_world_anchor_b() const { DRAFT_MAYBE_OR_ELSE(m_nativeHandlePtr, get_world_anchor_b(), {}); }
        inline Vector2f get_reaction_force(float inv_dt) const { DRAFT_MAYBE_OR_ELSE(m_nativeHandlePtr, get_reaction_force(inv_dt), {}); }
        inline float get_reaction_torque(float inv_dt) const { DRAFT_MAYBE_OR_ELSE(m_nativeHandlePtr, get_reaction_torque(inv_dt), 0.f); }
        inline bool get_collide_connected() const { DRAFT_MAYBE_OR_ELSE(m_nativeHandlePtr, get_collide_connected(), 0.f); }
        inline bool is_enabled() const { DRAFT_MAYBE_OR_ELSE(m_nativeHandlePtr, is_enabled(), 0.f); }
    };

    /**
     * @brief Lists every entity this entity is constrained to via a joint.
     */
    struct ConstrainedComponent {
        std::vector<Entity> constraints;
        
        DRAFT_REFLECTABLE(ConstrainedComponent, constraints)
    };

    // Specialized components
    struct DistanceJointComponent : public JointComponent<DistanceJointData> {
        inline float get_current_length() const { DRAFT_MAYBE_OR_ELSE(static_cast<DistanceJoint*>(m_nativeHandlePtr), get_current_length(), 0.f); }

        DRAFT_REFLECTABLE(DistanceJointComponent, entityA, entityB, collideConnected,
            anchorA, anchorB, length, minLength, maxLength, stiffness, damping)
    };

    struct FrictionJointComponent : public JointComponent<FrictionJointData> {
        DRAFT_REFLECTABLE(FrictionJointComponent, entityA, entityB, collideConnected,
            anchorA, anchorB, maxForce, maxTorque)
    };

    struct GearJointComponent : public JointComponent<GearJointData> {
        // joint1/joint2 stay unreflected, same reasoning as GearJointData itself.
        DRAFT_REFLECTABLE(GearJointComponent, entityA, entityB, collideConnected, ratio)
    };

    struct MotorJointComponent : public JointComponent<MotorJointData> {
        DRAFT_REFLECTABLE(MotorJointComponent, entityA, entityB, collideConnected,
            linearOffset, angularOffset, maxForce, maxTorque, correctionFactor)
    };

    struct MouseJointComponent : public JointComponent<MouseJointData> {
        DRAFT_REFLECTABLE(MouseJointComponent, entityA, entityB, collideConnected,
            target, maxForce, stiffness, damping)
    };

    struct PrismaticJointComponent : public JointComponent<PrismaticJointData> {
        inline float get_joint_translation() const { DRAFT_MAYBE_OR_ELSE(static_cast<PrismaticJoint*>(m_nativeHandlePtr), get_joint_translation(), 0.f); }

        DRAFT_REFLECTABLE(PrismaticJointComponent, entityA, entityB, collideConnected,
            anchorA, anchorB, localAxisA, referenceAngle, lowerTranslation, upperTranslation,
            maxMotorForce, motorSpeed, enableLimit, enableMotor)
    };

    struct PulleyJointComponent : public JointComponent<PulleyJointData> {
        inline float get_current_length_a() const { DRAFT_MAYBE_OR_ELSE(static_cast<PulleyJoint*>(m_nativeHandlePtr), get_current_length_a(), 0.f); }
        inline float get_current_length_b() const { DRAFT_MAYBE_OR_ELSE(static_cast<PulleyJoint*>(m_nativeHandlePtr), get_current_length_b(), 0.f); }

        DRAFT_REFLECTABLE(PulleyJointComponent, entityA, entityB, collideConnected,
            groundAnchorA, groundAnchorB, localAnchorA, localAnchorB, lengthA, lengthB, ratio)
    };

    struct RevoluteJointComponent : public JointComponent<RevoluteJointData> {
        DRAFT_REFLECTABLE(RevoluteJointComponent, entityA, entityB, collideConnected,
            localAnchorA, localAnchorB, referenceAngle, lowerAngle, upperAngle,
            maxMotorTorque, motorSpeed, enableLimit, enableMotor)
    };

    struct WeldJointComponent : public JointComponent<WeldJointData> {
        DRAFT_REFLECTABLE(WeldJointComponent, entityA, entityB, collideConnected,
            anchorA, anchorB, referenceAngle, stiffness, damping)
    };

    struct WheelJointComponent : public JointComponent<WheelJointData> {
        DRAFT_REFLECTABLE(WheelJointComponent, entityA, entityB, collideConnected,
            anchorA, anchorB, localAxis, lowerTranslation, upperTranslation,
            maxMotorTorque, motorSpeed, stiffness, damping, enableLimit, enableMotor)
    };

    // Aliasing for metaprogramming
    #define DRAFT_ALL_JOINT_TYPES DistanceJointComponent, FrictionJointComponent, GearJointComponent, MotorJointComponent, MouseJointComponent, PrismaticJointComponent, PulleyJointComponent, RevoluteJointComponent, WeldJointComponent, WheelJointComponent
    using JointComponentTypes = std::tuple<DRAFT_ALL_JOINT_TYPES>;

    // Factory
    template<typename T>
    std::pair<Entity, T&> create_joint_entity(Scene& scene, const T& data){
        Entity entity = scene.create_entity();
        return {entity, entity.add_component<T>(data)};
    }
};
