#pragma once

#include "draft/core/entity.hpp"
#include "draft/phys/joint.hpp"
#include "draft/phys/joint_def.hpp"
#include <tuple>
#include <type_traits>
#include <vector>

#define DRAFT_MAYBE_OR_ELSE(ptr, returnValue, failureValue) if(ptr) return ptr->returnValue; else return failureValue;
#define DRAFT_MAYBE(ptr, returnValue) if(ptr) return ptr->returnValue;

namespace Draft {
    // Base Components
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
    };

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

    struct ConstrainedComponent {
        // Holds a list of all the joint components which constraint the entitiy to another entitiy
        std::vector<Entity> constraints;
    };
    
    // Specialized components
    struct DistanceJointComponent : public JointComponent<DistanceJointData> {
        inline float get_current_length() const { DRAFT_MAYBE_OR_ELSE(static_cast<DistanceJoint*>(m_nativeHandlePtr), get_current_length(), 0.f); }
    };

    struct FrictionJointComponent : public JointComponent<FrictionJointData> {
    };

    struct GearJointComponent : public JointComponent<GearJointData> {
    };

    struct MotorJointComponent : public JointComponent<MotorJointData> {
    };

    struct MouseJointComponent : public JointComponent<MouseJointData> {
    };

    struct PrismaticJointComponent : public JointComponent<PrismaticJointData> {
    };

    struct PulleyJointComponent : public JointComponent<PulleyJointData> {
    };

    struct RevoluteJointComponent : public JointComponent<RevoluteJointData> {
    };

    struct WeldJointComponent : public JointComponent<WeldJointData> {
    };

    struct WheelJointComponent : public JointComponent<WheelJointData> {
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