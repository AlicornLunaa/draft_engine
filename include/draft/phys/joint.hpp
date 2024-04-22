#pragma once

#include "draft/math/vector2.hpp"

#include <memory>

class b2GearJointDef;

namespace Draft {
    // Forward declarations
    class World;
    class RigidBody;
    class GearJointDef;

    // Base class
    class Joint {
    public:
        // Enumerators
        enum class Type { UNKNOWN, DISTANCE, REVOLUTE, PRISMATIC, PULLEY, GEAR, MOUSE, WHEEL, WELD, ROPE, FRICTION, MOTOR };

        // Constructors
        Joint(const Joint& other) = delete;
        virtual ~Joint();

        // Friends
        friend class World;
        friend class RigidBody;
        friend b2GearJointDef jointdef_to_b2(const GearJointDef& def);

        // Functions
        Type get_type() const;
        World* get_world();
        RigidBody* get_body_a();
        RigidBody* get_body_b();
        virtual Vector2f get_world_anchor_a() const;
        virtual Vector2f get_world_anchor_b() const;
        virtual Vector2f get_reaction_force(float inv_dt) const;
        virtual float get_reaction_torque(float inv_dt) const;
        bool is_enabled() const;
        bool get_collide_connected() const;
        void destroy();

    protected:
        // Private functions
        void* get_joint_ptr();
        const void* get_joint_ptr() const;

        // Constructors
        Joint(World* world, RigidBody* body1, RigidBody* body2, void* jointPtr, Type type);
        
    private:
        // Variables
        World* world = nullptr;
        RigidBody* bodyPtr1 = nullptr;
        RigidBody* bodyPtr2 = nullptr;
        const Type type = Type::UNKNOWN;

        // pImpl
        struct Impl;
        std::unique_ptr<Impl> ptr;
    };

    // Distance joint
    class DistanceJoint : public Joint {
    public:
        // Functions
        Vector2f get_local_anchor_a() const;
        Vector2f get_local_anchor_b() const;

        float get_length() const;
        float get_min_length() const;
        float get_max_length() const;
        float get_current_length() const;
        float get_stiffness() const;
        float get_damping() const;

        void set_length(float length);
        void set_min_length(float length);
        void set_max_length(float length);
        void set_stiffness(float stiffness);
        void set_damping(float damping);

        // Friends
        friend class World;
        friend class RigidBody;

    protected:
        // Constructors
        DistanceJoint(World* world, RigidBody* body1, RigidBody* body2, void* jointPtr);
    };

    // Friction joint
    class FrictionJoint : public Joint {
    public:
        // Functions
        Vector2f get_local_anchor_a() const;
        Vector2f get_local_anchor_b() const;

        float get_max_force() const;
        float get_max_torque() const;

        void set_max_force(float force);
        void set_max_torque(float torque);

        // Friends
        friend class World;
        friend class RigidBody;

    protected:
        // Constructors
        FrictionJoint(World* world, RigidBody* body1, RigidBody* body2, void* jointPtr);
    };

    // Gear joint
    class GearJoint : public Joint {
    private:
        // Variables
        Joint* joint1;
        Joint* joint2;

    public:
        // Functions
        const Joint* get_joint_1() const;
        const Joint* get_joint_2() const;
        float get_ratio() const;
        void set_ratio(float ratio);

        // Friends
        friend class World;
        friend class RigidBody;

    protected:
        // Constructors
        GearJoint(World* world, RigidBody* body1, RigidBody* body2, Joint* joint1, Joint* joint2, void* jointPtr);
    };

    // Motor joint
    class MotorJoint : public Joint {
    public:
        // Functions
        Vector2f get_linear_offset() const;
        float get_angular_offset() const;
        float get_max_force() const;
        float get_max_torque() const;
        float get_correction_factor() const;

        void set_linear_offset(const Vector2f& offset);
        void set_angular_offset(float offset);
        void set_max_force(float force);
        void set_max_torque(float torque);
        void set_correction_factor(float force);

        // Friends
        friend class World;
        friend class RigidBody;

    protected:
        // Constructors
        MotorJoint(World* world, RigidBody* body1, RigidBody* body2, void* jointPtr);
    };

    // Mouse joint
    class MouseJoint : public Joint {
    public:
        // Functions
        Vector2f get_target() const;
        float get_max_force() const;
        float get_stiffness() const;
        float get_damping() const;

        void set_target(const Vector2f& target);
        void set_max_force(float force);
        void set_stiffness(float stiffness);
        void set_damping(float damping);

        // Friends
        friend class World;
        friend class RigidBody;

    protected:
        // Constructors
        MouseJoint(World* world, RigidBody* body1, RigidBody* body2, void* jointPtr);
    };

    // Prismatic joint
    class PrismaticJoint : public Joint {
    public:
        // Functions
        Vector2f get_local_anchor_a() const;
        Vector2f get_local_anchor_b() const;
        Vector2f get_local_axis() const;
        float get_reference_angle() const;
        float get_joint_translation() const;
        float get_joint_speed() const;
        float get_lower_limit() const;
        float get_upper_limit() const;
        float get_motor_speed() const;
        float get_max_motor_force() const;
        float get_motor_force(float inv_dt) const;
        bool is_limit_enabled() const;
        bool is_motor_enabled() const;

        void set_limits(float lower, float upper);
        void set_motor_speed(float speed);
        void set_max_motor_force(float force);
        void enable_limit(bool flag);
        void enable_motor(bool flag);

        // Friends
        friend class World;
        friend class RigidBody;
        
    protected:
        // Constructors
        PrismaticJoint(World* world, RigidBody* body1, RigidBody* body2, void* jointPtr);
    };

    // Pulley joint
    class PulleyJoint : public Joint {
    public:
        // Functions
        Vector2f get_ground_anchor_a() const;
        Vector2f get_ground_anchor_b() const;
        float get_length_a() const;
        float get_length_b() const;
        float get_ratio() const;
        float get_current_length_a() const;
        float get_current_length_b() const;

        // Friends
        friend class World;
        friend class RigidBody;
        
    protected:
        // Constructors
        PulleyJoint(World* world, RigidBody* body1, RigidBody* body2, void* jointPtr);
    };

    // Revolute joint
    class RevoluteJoint : public Joint {
    public:
        // Functions
        Vector2f get_local_anchor_a() const;
        Vector2f get_local_anchor_b() const;
        float get_reference_angle() const;
        float get_joint_angle() const;
        float get_joint_speed() const;
        float get_lower_limit() const;
        float get_upper_limit() const;
        float get_motor_speed() const;
        float get_max_motor_torque() const;
        float get_motor_torque(float inv_dt) const;
        bool is_limit_enabled() const;
        bool is_motor_enabled() const;

        void enable_limit(bool flag);
        void enable_motor(bool flag);
        void set_limits(float lower, float upper);
        void set_motor_speed(float speed);
        void set_max_motor_torque(float torque);

        // Friends
        friend class World;
        friend class RigidBody;
        
    protected:
        // Constructors
        RevoluteJoint(World* world, RigidBody* body1, RigidBody* body2, void* jointPtr);
    };

    // Weld joint
    class WeldJoint : public Joint {
    public:
        // Functions
        Vector2f get_local_anchor_a() const;
        Vector2f get_local_anchor_b() const;
        float get_reference_angle() const;
        float get_stiffness() const;
        float get_damping() const;

        void set_stiffness(float hz);
        void set_damping(float damping);

        // Friends
        friend class World;
        friend class RigidBody;
        
    protected:
        // Constructors
        WeldJoint(World* world, RigidBody* body1, RigidBody* body2, void* jointPtr);
    };

    // Wheel joint
    class WheelJoint : public Joint {
    public:
        // Functions
        Vector2f get_local_anchor_a() const;
        Vector2f get_local_anchor_b() const;
        Vector2f get_local_axis() const;
        float get_joint_translation() const;
        float get_joint_linear_speed() const;
        float get_joint_angle() const;
        float get_joint_angular_speed() const;
        float get_lower_limit() const;
        float get_upper_limit() const;
        float get_motor_speed() const;
        float get_max_motor_torque() const;
        float get_motor_torque(float inv_dt) const;
        float get_stiffness() const;
        float get_damping() const;
        bool is_limit_enabled() const;
        bool is_motor_enabled() const;

        void enable_limit(bool flag);
        void enable_motor(bool flag);
        void set_limits(float lower, float upper);
        void set_motor_speed(float speed);
        void set_max_motor_torque(float torque);
        void set_stiffness(float stiffness);
        void set_damping(float damping);

        // Friends
        friend class World;
        friend class RigidBody;
        
    protected:
        // Constructors
        WheelJoint(World* world, RigidBody* body1, RigidBody* body2, void* jointPtr);
    };
};