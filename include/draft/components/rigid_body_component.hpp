#pragma once

#include "draft/math/glm.hpp"
#include "draft/phys/fixture.hpp"
#include "draft/phys/rigid_body.hpp"

namespace Draft {
    /// RigidBody pointer for physics
    struct NativeBodyComponent {
        RigidBody* bodyPtr = nullptr;
        
        Vector2f deltaP = {};
        float deltaR = 0.f;

        RigidBody::BodyType deltaType = RigidBody::STATIC;
        Vector2f deltaLinearVelocity = {};
        float deltaAngularVelocity = 0.f;
        float deltaLinearDamping = 0.f;
        float deltaAngularDamping = 0.f;
        bool deltaAllowSleep = true;
        bool deltaAwake = true;
        bool deltaEnabled = true;
        bool deltaBullet = false;
        bool deltaFixedRotation = false;
        float deltaGravityScale = 1.f;

        RigidBody* operator->(){ return bodyPtr; }
        operator RigidBody& () { return *bodyPtr; }
        operator RigidBody* () { return bodyPtr; }
        operator const RigidBody* () const { return bodyPtr; }
        operator bool () const { return is_valid(); }

        bool is_valid() const { return bodyPtr != nullptr && bodyPtr->is_valid(); }
    };

    /// Abstracted rigidbody component
    struct RigidBodyComponent {
        // State of the rigidbody, will be synced to the native body automatically
        RigidBody::BodyType type = RigidBody::STATIC;
        Vector2f linearVelocity = { 0, 0 };
        float angularVelocity = 0.f;
        float linearDamping = 0.f;
        float angularDamping = 0.f;
        bool allowSleep = true;
        bool awake = true;
        bool enabled = true;
        bool bullet = false;
        bool fixedRotation = false;
        float gravityScale = 1.f;

        // Native handle for the indirection since the RigidBody is handled with the native component
        RigidBody* m_nativeHandlePtr = nullptr;

        // Functions
        inline Vector2f get_world_center() const { if(m_nativeHandlePtr) return m_nativeHandlePtr->get_world_center(); else return {}; }
        inline Vector2f get_local_center() const { if(m_nativeHandlePtr) return m_nativeHandlePtr->get_local_center(); else return {}; }
        inline float get_inertia() const { if(m_nativeHandlePtr) return m_nativeHandlePtr->get_inertia(); else return 0.f; }
        inline float get_mass() const { if(m_nativeHandlePtr) return m_nativeHandlePtr->get_mass(); else return 0.f; }
        inline bool is_touching(const RigidBodyComponent& other) const { if(m_nativeHandlePtr && other.m_nativeHandlePtr) return m_nativeHandlePtr->is_touching(*other.m_nativeHandlePtr); else return false; }

        inline Vector2f get_world_point(const Vector2f& localPoint) const { if(m_nativeHandlePtr) return m_nativeHandlePtr->get_world_point(localPoint); else return {}; }
        inline Vector2f get_world_vector(const Vector2f& localVector) const { if(m_nativeHandlePtr) return m_nativeHandlePtr->get_world_vector(localVector); else return {}; }
        inline Vector2f get_local_point(const Vector2f& worldPoint) const { if(m_nativeHandlePtr) return m_nativeHandlePtr->get_local_point(worldPoint); else return {}; }
        inline Vector2f get_local_vector(const Vector2f& worldVector) const { if(m_nativeHandlePtr) return m_nativeHandlePtr->get_local_vector(worldVector); else return {}; }
        inline Vector2f get_linear_velocity_from_world_point(const Vector2f& worldPoint) const { if(m_nativeHandlePtr) return m_nativeHandlePtr->get_linear_velocity_from_world_point(worldPoint); else return {}; }
        inline Vector2f get_linear_velocity_from_local_point(const Vector2f& localPoint) const { if(m_nativeHandlePtr) return m_nativeHandlePtr->get_linear_velocity_from_local_point(localPoint); else return {}; }
        
        inline void apply_force(const Vector2f& force, const Vector2f& point, bool wake = true) { if(m_nativeHandlePtr) m_nativeHandlePtr->apply_force(force, point, wake); }
    	inline void apply_force(const Vector2f& force, bool wake = true) { if(m_nativeHandlePtr) m_nativeHandlePtr->apply_force(force, wake); }
    	inline void apply_torque(float torque, bool wake = true) { if(m_nativeHandlePtr) m_nativeHandlePtr->apply_torque(torque, wake); }
    	inline void apply_linear_impulse(const Vector2f& impulse, const Vector2f& point, bool wake = true) { if(m_nativeHandlePtr) m_nativeHandlePtr->apply_linear_impulse(impulse, point, wake); }
    	inline void apply_linear_impulse(const Vector2f& impulse, bool wake = true) { if(m_nativeHandlePtr) m_nativeHandlePtr->apply_linear_impulse(impulse, wake); }
    	inline void apply_angular_impulse(float impulse, bool wake = true) { if(m_nativeHandlePtr) m_nativeHandlePtr->apply_angular_impulse(impulse, wake); }
    };

    /// Forces components
    struct TorqueComponent {
        float torque = 0.f;
        bool wake = true;
    };

    struct ForceComponent {
        Vector2f force;
        Vector2f point;
        bool wake = true;
    };

    struct ImpulseComponent {
        Vector2f force;
        Vector2f point;
        float angular = 0.f;
        bool wake = true;
    };

    struct ContinuousTorqueComponent {
        float torque = 0.f;
        bool wake = true;
        bool enabled = false;
    };

    struct ContinuousForceComponent {
        Vector2f force;
        Vector2f point;
        bool wake = true;
        bool enabled = true;
    };

    struct ContinuousImpulseComponent {
        Vector2f force;
        Vector2f point;
        float angular = 0.f;
        bool wake = true;
        bool enabled = true;
    };
}