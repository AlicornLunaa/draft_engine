#include "draft/phys/world.hpp"
#include "draft/phys/joint.hpp"
#include "draft/math/vector2.hpp"
#include "draft/math/vector2_p.hpp"

#include "box2d/b2_joint.h"
#include "box2d/b2_body.h"
#include "box2d/b2_distance_joint.h"
#include "box2d/b2_friction_joint.h"
#include "box2d/b2_gear_joint.h"
#include "box2d/b2_motor_joint.h"
#include "box2d/b2_mouse_joint.h"
#include "box2d/b2_prismatic_joint.h"
#include "box2d/b2_pulley_joint.h"
#include "box2d/b2_revolute_joint.h"
#include "box2d/b2_weld_joint.h"
#include "box2d/b2_wheel_joint.h"

#include <memory>

namespace Draft {
    // Start Base Joint
    // pImpl
    struct Joint::Impl {
        b2Joint* joint;
    };

    // Private functions
    void* Joint::get_joint_ptr(){ return (void*)ptr->joint; }
    const void* Joint::get_joint_ptr() const { return (void*)ptr->joint; }

    // Constructors
    Joint::Joint(World* world, RigidBody* body1, RigidBody* body2, void* jointPtr, Type type) : ptr(std::make_unique<Impl>()), type(type) {
        this->world = world;
        bodyPtr1 = body1;
        bodyPtr2 = body2;
        ptr->joint = (b2Joint*)jointPtr;
    }

    Joint::~Joint(){}

    // Functions
    Joint::Type Joint::get_type() const { return type; };
    World* Joint::get_world(){ return world; }
    RigidBody* Joint::get_body_a(){ return bodyPtr1; }
    RigidBody* Joint::get_body_b(){ return bodyPtr2; }

    Vector2f Joint::get_world_anchor_a() const { return b2_to_vector<float>(ptr->joint->GetAnchorA()); }
    Vector2f Joint::get_world_anchor_b() const { return b2_to_vector<float>(ptr->joint->GetAnchorB()); }
    Vector2f Joint::get_reaction_force(float inv_dt) const { return b2_to_vector<float>(ptr->joint->GetReactionForce(inv_dt)); };
    float Joint::get_reaction_torque(float inv_dt) const { return ptr->joint->GetReactionTorque(inv_dt); };
    
    bool Joint::is_enabled() const { return ptr->joint->IsEnabled(); }
    bool Joint::get_collide_connected() const { return ptr->joint->GetCollideConnected(); }

    void Joint::destroy(){ world->destroy_joint(this); }
    // End Base Joint
    
    // Start Distance Joint
    Vector2f DistanceJoint::get_local_anchor_a() const { return b2_to_vector<float>(static_cast<const b2DistanceJoint*>(get_joint_ptr())->GetLocalAnchorA()); }
    Vector2f DistanceJoint::get_local_anchor_b() const { return b2_to_vector<float>(static_cast<const b2DistanceJoint*>(get_joint_ptr())->GetLocalAnchorB()); }

    float DistanceJoint::get_length() const { return static_cast<const b2DistanceJoint*>(get_joint_ptr())->GetLength(); }
    float DistanceJoint::get_min_length() const { return static_cast<const b2DistanceJoint*>(get_joint_ptr())->GetMinLength(); }
    float DistanceJoint::get_max_length() const { return static_cast<const b2DistanceJoint*>(get_joint_ptr())->GetMaxLength(); }
    float DistanceJoint::get_current_length() const { return static_cast<const b2DistanceJoint*>(get_joint_ptr())->GetCurrentLength(); }
    float DistanceJoint::get_stiffness() const { return static_cast<const b2DistanceJoint*>(get_joint_ptr())->GetStiffness(); }
    float DistanceJoint::get_damping() const { return static_cast<const b2DistanceJoint*>(get_joint_ptr())->GetDamping(); }

    void DistanceJoint::set_length(float length){ static_cast<b2DistanceJoint*>(get_joint_ptr())->SetLength(length); }
    void DistanceJoint::set_min_length(float length){ static_cast<b2DistanceJoint*>(get_joint_ptr())->SetMinLength(length); }
    void DistanceJoint::set_max_length(float length){ static_cast<b2DistanceJoint*>(get_joint_ptr())->SetMaxLength(length); }
    void DistanceJoint::set_stiffness(float stiffness){ static_cast<b2DistanceJoint*>(get_joint_ptr())->SetStiffness(stiffness); }
    void DistanceJoint::set_damping(float damping){ static_cast<b2DistanceJoint*>(get_joint_ptr())->SetDamping(damping); }

    DistanceJoint::DistanceJoint(World* world, RigidBody* body1, RigidBody* body2, void* jointPtr) : Joint(world, body1, body2, jointPtr, Type::DISTANCE) {}
    // End Distance Joint
    
    // Start Friction Joint
    Vector2f FrictionJoint::get_local_anchor_a() const { return b2_to_vector<float>(static_cast<const b2FrictionJoint*>(get_joint_ptr())->GetLocalAnchorA()); }
    Vector2f FrictionJoint::get_local_anchor_b() const { return b2_to_vector<float>(static_cast<const b2FrictionJoint*>(get_joint_ptr())->GetLocalAnchorB()); }

    float FrictionJoint::get_max_force() const { return static_cast<const b2FrictionJoint*>(get_joint_ptr())->GetMaxForce(); }
    float FrictionJoint::get_max_torque() const { return static_cast<const b2FrictionJoint*>(get_joint_ptr())->GetMaxTorque(); }

    void FrictionJoint::set_max_force(float force){ static_cast<b2FrictionJoint*>(get_joint_ptr())->SetMaxForce(force); }
    void FrictionJoint::set_max_torque(float torque){ static_cast<b2FrictionJoint*>(get_joint_ptr())->SetMaxTorque(torque); }

    FrictionJoint::FrictionJoint(World* world, RigidBody* body1, RigidBody* body2, void* jointPtr) : Joint(world, body1, body2, jointPtr, Type::FRICTION) {}
    // End Friction Joint
    
    // Start Gear Joint
    const Joint* GearJoint::get_joint_1() const { return joint1; }
    const Joint* GearJoint::get_joint_2() const { return joint2; }
    float GearJoint::get_ratio() const { return static_cast<const b2GearJoint*>(get_joint_ptr())->GetRatio(); }
    void GearJoint::set_ratio(float ratio){ static_cast<b2GearJoint*>(get_joint_ptr())->SetRatio(ratio); }

    GearJoint::GearJoint(World* world, RigidBody* body1, RigidBody* body2, Joint* joint1, Joint* joint2, void* jointPtr) : Joint(world, body1, body2, jointPtr, Type::GEAR) {
        this->joint1 = joint1;
        this->joint2 = joint2;
    }
    // End Gear Joint
    
    // Start Motor Joint
    Vector2f MotorJoint::get_linear_offset() const { return b2_to_vector<float>(static_cast<const b2MotorJoint*>(get_joint_ptr())->GetLinearOffset()); }
    float MotorJoint::get_angular_offset() const { return static_cast<const b2MotorJoint*>(get_joint_ptr())->GetAngularOffset(); }
    float MotorJoint::get_max_force() const { return static_cast<const b2MotorJoint*>(get_joint_ptr())->GetMaxForce(); }
    float MotorJoint::get_max_torque() const { return static_cast<const b2MotorJoint*>(get_joint_ptr())->GetMaxTorque(); }
    float MotorJoint::get_correction_factor() const { return static_cast<const b2MotorJoint*>(get_joint_ptr())->GetCorrectionFactor(); }

    void MotorJoint::set_linear_offset(const Vector2f& offset){ static_cast<b2MotorJoint*>(get_joint_ptr())->SetLinearOffset(vector_to_b2(offset)); }
    void MotorJoint::set_angular_offset(float offset){ static_cast<b2MotorJoint*>(get_joint_ptr())->SetAngularOffset(offset); }
    void MotorJoint::set_max_force(float force){ static_cast<b2MotorJoint*>(get_joint_ptr())->SetMaxForce(force); }
    void MotorJoint::set_max_torque(float torque){ static_cast<b2MotorJoint*>(get_joint_ptr())->SetMaxTorque(torque); }
    void MotorJoint::set_correction_factor(float force){ static_cast<b2MotorJoint*>(get_joint_ptr())->SetCorrectionFactor(force); }

    MotorJoint::MotorJoint(World* world, RigidBody* body1, RigidBody* body2, void* jointPtr) : Joint(world, body1, body2, jointPtr, Type::MOTOR) {}
    // End Motor Joint

    // Start Mouse Joint
    Vector2f MouseJoint::get_target() const { return b2_to_vector<float>(static_cast<const b2MouseJoint*>(get_joint_ptr())->GetTarget()); }
    float MouseJoint::get_max_force() const { return static_cast<const b2MouseJoint*>(get_joint_ptr())->GetMaxForce(); }
    float MouseJoint::get_stiffness() const { return static_cast<const b2MouseJoint*>(get_joint_ptr())->GetStiffness(); }
    float MouseJoint::get_damping() const { return static_cast<const b2MouseJoint*>(get_joint_ptr())->GetDamping(); }

    void MouseJoint::set_target(const Vector2f& target){ return static_cast<b2MouseJoint*>(get_joint_ptr())->SetTarget(vector_to_b2(target)); }
    void MouseJoint::set_max_force(float force){ return static_cast<b2MouseJoint*>(get_joint_ptr())->SetMaxForce(force); }
    void MouseJoint::set_stiffness(float stiffness){ return static_cast<b2MouseJoint*>(get_joint_ptr())->SetStiffness(stiffness); }
    void MouseJoint::set_damping(float damping){ return static_cast<b2MouseJoint*>(get_joint_ptr())->SetDamping(damping); }

    MouseJoint::MouseJoint(World* world, RigidBody* body1, RigidBody* body2, void* jointPtr) : Joint(world, body1, body2, jointPtr, Type::MOUSE) {}
    // End Mouse Joint

    // Start Prismatic Joint
    Vector2f PrismaticJoint::get_local_anchor_a() const { return b2_to_vector<float>(static_cast<const b2PrismaticJoint*>(get_joint_ptr())->GetLocalAnchorA()); }
    Vector2f PrismaticJoint::get_local_anchor_b() const { return b2_to_vector<float>(static_cast<const b2PrismaticJoint*>(get_joint_ptr())->GetLocalAnchorB()); }
    Vector2f PrismaticJoint::get_local_axis() const { return b2_to_vector<float>(static_cast<const b2PrismaticJoint*>(get_joint_ptr())->GetLocalAxisA()); }
    float PrismaticJoint::get_reference_angle() const { return static_cast<const b2PrismaticJoint*>(get_joint_ptr())->GetReferenceAngle(); }
    float PrismaticJoint::get_joint_translation() const { return static_cast<const b2PrismaticJoint*>(get_joint_ptr())->GetJointTranslation(); }
    float PrismaticJoint::get_joint_speed() const { return static_cast<const b2PrismaticJoint*>(get_joint_ptr())->GetJointSpeed(); }
    float PrismaticJoint::get_lower_limit() const { return static_cast<const b2PrismaticJoint*>(get_joint_ptr())->GetLowerLimit(); }
    float PrismaticJoint::get_upper_limit() const { return static_cast<const b2PrismaticJoint*>(get_joint_ptr())->GetUpperLimit(); }
    float PrismaticJoint::get_motor_speed() const { return static_cast<const b2PrismaticJoint*>(get_joint_ptr())->GetMotorSpeed(); }
    float PrismaticJoint::get_max_motor_force() const { return static_cast<const b2PrismaticJoint*>(get_joint_ptr())->GetMaxMotorForce(); }
    float PrismaticJoint::get_motor_force(float inv_dt) const { return static_cast<const b2PrismaticJoint*>(get_joint_ptr())->GetMotorForce(inv_dt); }
    bool PrismaticJoint::is_limit_enabled() const { return static_cast<const b2PrismaticJoint*>(get_joint_ptr())->IsLimitEnabled(); }
    bool PrismaticJoint::is_motor_enabled() const { return static_cast<const b2PrismaticJoint*>(get_joint_ptr())->IsMotorEnabled(); }

    void PrismaticJoint::set_limits(float lower, float upper){ static_cast<b2PrismaticJoint*>(get_joint_ptr())->SetLimits(lower, upper); }
    void PrismaticJoint::set_motor_speed(float speed){ static_cast<b2PrismaticJoint*>(get_joint_ptr())->SetMotorSpeed(speed); }
    void PrismaticJoint::set_max_motor_force(float force){ static_cast<b2PrismaticJoint*>(get_joint_ptr())->SetMaxMotorForce(force); }
    void PrismaticJoint::enable_limit(bool flag){ static_cast<b2PrismaticJoint*>(get_joint_ptr())->EnableLimit(flag); }
    void PrismaticJoint::enable_motor(bool flag){ static_cast<b2PrismaticJoint*>(get_joint_ptr())->EnableMotor(flag); }
            
    PrismaticJoint::PrismaticJoint(World* world, RigidBody* body1, RigidBody* body2, void* jointPtr) : Joint(world, body1, body2, jointPtr, Type::PRISMATIC) {}
    // End Prismatic Joint

    // Start Pulley Joint
    Vector2f PulleyJoint::get_ground_anchor_a() const { return b2_to_vector<float>(static_cast<const b2PulleyJoint*>(get_joint_ptr())->GetGroundAnchorA()); }
    Vector2f PulleyJoint::get_ground_anchor_b() const { return b2_to_vector<float>(static_cast<const b2PulleyJoint*>(get_joint_ptr())->GetGroundAnchorB()); }
    float PulleyJoint::get_length_a() const { return static_cast<const b2PulleyJoint*>(get_joint_ptr())->GetLengthA(); }
    float PulleyJoint::get_length_b() const { return static_cast<const b2PulleyJoint*>(get_joint_ptr())->GetLengthB(); }
    float PulleyJoint::get_ratio() const { return static_cast<const b2PulleyJoint*>(get_joint_ptr())->GetRatio(); }
    float PulleyJoint::get_current_length_a() const { return static_cast<const b2PulleyJoint*>(get_joint_ptr())->GetCurrentLengthA(); }
    float PulleyJoint::get_current_length_b() const { return static_cast<const b2PulleyJoint*>(get_joint_ptr())->GetCurrentLengthB(); }
            
    PulleyJoint::PulleyJoint(World* world, RigidBody* body1, RigidBody* body2, void* jointPtr) : Joint(world, body1, body2, jointPtr, Type::PULLEY) {}
    // End Pulley Joint

    // Start Revolute Joint
    Vector2f RevoluteJoint::get_local_anchor_a() const { return b2_to_vector<float>(static_cast<const b2RevoluteJoint*>(get_joint_ptr())->GetLocalAnchorA()); }
    Vector2f RevoluteJoint::get_local_anchor_b() const { return b2_to_vector<float>(static_cast<const b2RevoluteJoint*>(get_joint_ptr())->GetLocalAnchorB()); }
    float RevoluteJoint::get_reference_angle() const { return static_cast<const b2RevoluteJoint*>(get_joint_ptr())->GetReferenceAngle(); }
    float RevoluteJoint::get_joint_angle() const { return static_cast<const b2RevoluteJoint*>(get_joint_ptr())->GetJointAngle(); }
    float RevoluteJoint::get_joint_speed() const { return static_cast<const b2RevoluteJoint*>(get_joint_ptr())->GetJointSpeed(); }
    float RevoluteJoint::get_lower_limit() const { return static_cast<const b2RevoluteJoint*>(get_joint_ptr())->GetLowerLimit(); }
    float RevoluteJoint::get_upper_limit() const { return static_cast<const b2RevoluteJoint*>(get_joint_ptr())->GetUpperLimit(); }
    float RevoluteJoint::get_motor_speed() const { return static_cast<const b2RevoluteJoint*>(get_joint_ptr())->GetMotorSpeed(); }
    float RevoluteJoint::get_max_motor_torque() const { return static_cast<const b2RevoluteJoint*>(get_joint_ptr())->GetMaxMotorTorque(); }
    float RevoluteJoint::get_motor_torque(float inv_dt) const { return static_cast<const b2RevoluteJoint*>(get_joint_ptr())->GetMotorTorque(inv_dt); }
    bool RevoluteJoint::is_limit_enabled() const { return static_cast<const b2RevoluteJoint*>(get_joint_ptr())->IsLimitEnabled(); }
    bool RevoluteJoint::is_motor_enabled() const { return static_cast<const b2RevoluteJoint*>(get_joint_ptr())->IsMotorEnabled(); }

    void RevoluteJoint::enable_limit(bool flag){ static_cast<b2RevoluteJoint*>(get_joint_ptr())->EnableLimit(flag); }
    void RevoluteJoint::enable_motor(bool flag){ static_cast<b2RevoluteJoint*>(get_joint_ptr())->EnableMotor(flag); }
    void RevoluteJoint::set_limits(float lower, float upper){ static_cast<b2RevoluteJoint*>(get_joint_ptr())->SetLimits(lower, upper); }
    void RevoluteJoint::set_motor_speed(float speed){ static_cast<b2RevoluteJoint*>(get_joint_ptr())->SetMotorSpeed(speed); }
    void RevoluteJoint::set_max_motor_torque(float torque){ static_cast<b2RevoluteJoint*>(get_joint_ptr())->SetMaxMotorTorque(torque); }
            
    RevoluteJoint::RevoluteJoint(World* world, RigidBody* body1, RigidBody* body2, void* jointPtr) : Joint(world, body1, body2, jointPtr, Type::REVOLUTE) {}
    // End Revolute Joint

    // Start Weld Joint
    Vector2f WeldJoint::get_local_anchor_a() const { return b2_to_vector<float>(static_cast<const b2WeldJoint*>(get_joint_ptr())->GetLocalAnchorA()); }
    Vector2f WeldJoint::get_local_anchor_b() const { return b2_to_vector<float>(static_cast<const b2WeldJoint*>(get_joint_ptr())->GetLocalAnchorB()); }
    float WeldJoint::get_reference_angle() const { return static_cast<const b2WeldJoint*>(get_joint_ptr())->GetReferenceAngle(); }
    float WeldJoint::get_stiffness() const { return static_cast<const b2WeldJoint*>(get_joint_ptr())->GetStiffness(); }
    float WeldJoint::get_damping() const { return static_cast<const b2WeldJoint*>(get_joint_ptr())->GetDamping(); }

    void WeldJoint::set_stiffness(float hz){ static_cast<b2WeldJoint*>(get_joint_ptr())->SetStiffness(hz); }
    void WeldJoint::set_damping(float damping){ static_cast<b2WeldJoint*>(get_joint_ptr())->SetDamping(damping); }
            
    WeldJoint::WeldJoint(World* world, RigidBody* body1, RigidBody* body2, void* jointPtr) : Joint(world, body1, body2, jointPtr, Type::WELD) {}
    // End Weld Joint

    // Start Wheel Joint
    Vector2f WheelJoint::get_local_anchor_a() const { return b2_to_vector<float>(static_cast<const b2WheelJoint*>(get_joint_ptr())->GetLocalAnchorA()); }
    Vector2f WheelJoint::get_local_anchor_b() const { return b2_to_vector<float>(static_cast<const b2WheelJoint*>(get_joint_ptr())->GetLocalAnchorB()); }
    Vector2f WheelJoint::get_local_axis() const { return b2_to_vector<float>(static_cast<const b2WheelJoint*>(get_joint_ptr())->GetLocalAxisA()); }
    float WheelJoint::get_joint_translation() const { return static_cast<const b2WheelJoint*>(get_joint_ptr())->GetJointTranslation(); }
    float WheelJoint::get_joint_linear_speed() const { return static_cast<const b2WheelJoint*>(get_joint_ptr())->GetJointLinearSpeed(); }
    float WheelJoint::get_joint_angle() const { return static_cast<const b2WheelJoint*>(get_joint_ptr())->GetJointAngle(); }
    float WheelJoint::get_joint_angular_speed() const { return static_cast<const b2WheelJoint*>(get_joint_ptr())->GetJointAngularSpeed(); }
    float WheelJoint::get_lower_limit() const { return static_cast<const b2WheelJoint*>(get_joint_ptr())->GetLowerLimit(); }
    float WheelJoint::get_upper_limit() const { return static_cast<const b2WheelJoint*>(get_joint_ptr())->GetUpperLimit(); }
    float WheelJoint::get_motor_speed() const { return static_cast<const b2WheelJoint*>(get_joint_ptr())->GetMotorSpeed(); }
    float WheelJoint::get_max_motor_torque() const { return static_cast<const b2WheelJoint*>(get_joint_ptr())->GetMaxMotorTorque(); }
    float WheelJoint::get_motor_torque(float inv_dt) const { return static_cast<const b2WheelJoint*>(get_joint_ptr())->GetMotorTorque(inv_dt); }
    float WheelJoint::get_stiffness() const { return static_cast<const b2WheelJoint*>(get_joint_ptr())->GetStiffness(); }
    float WheelJoint::get_damping() const { return static_cast<const b2WheelJoint*>(get_joint_ptr())->GetDamping(); }
    bool WheelJoint::is_limit_enabled() const { return static_cast<const b2WheelJoint*>(get_joint_ptr())->IsLimitEnabled(); }
    bool WheelJoint::is_motor_enabled() const { return static_cast<const b2WheelJoint*>(get_joint_ptr())->IsMotorEnabled(); }

    void WheelJoint::enable_limit(bool flag){ static_cast<b2WheelJoint*>(get_joint_ptr())->EnableLimit(flag); }
    void WheelJoint::enable_motor(bool flag){ static_cast<b2WheelJoint*>(get_joint_ptr())->EnableMotor(flag); }
    void WheelJoint::set_limits(float lower, float upper){ static_cast<b2WheelJoint*>(get_joint_ptr())->SetLimits(lower, upper); }
    void WheelJoint::set_motor_speed(float speed){ static_cast<b2WheelJoint*>(get_joint_ptr())->SetMotorSpeed(speed); }
    void WheelJoint::set_max_motor_torque(float torque){ static_cast<b2WheelJoint*>(get_joint_ptr())->SetMaxMotorTorque(torque); }
    void WheelJoint::set_stiffness(float stiffness){ static_cast<b2WheelJoint*>(get_joint_ptr())->SetStiffness(stiffness); }
    void WheelJoint::set_damping(float damping){ static_cast<b2WheelJoint*>(get_joint_ptr())->SetDamping(damping); }
            
    WheelJoint::WheelJoint(World* world, RigidBody* body1, RigidBody* body2, void* jointPtr) : Joint(world, body1, body2, jointPtr, Type::WHEEL) {}
    // End Wheel Joint
};