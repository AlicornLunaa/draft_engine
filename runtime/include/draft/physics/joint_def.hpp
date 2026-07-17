#pragma once

#include "draft/physics/joint.hpp"
#include "draft/physics/joint_data.hpp"

namespace Draft {
    // Forward declarations
    class RigidBody;

    // Structs
    struct JointDef : virtual public GenericJointData {
        // Variables
        Joint::Type type = Joint::Type::UNKNOWN;
        RigidBody* bodyA = nullptr;
        RigidBody* bodyB = nullptr;

        // Constructor
        JointDef(Joint::Type type, RigidBody* body1, RigidBody* body2, bool collide = false){
            this->type = type;
            bodyA = body1;
            bodyB = body2;
            collideConnected = collide;
        }
    };

    struct DistanceJointDef : public JointDef, public DistanceJointData {
        DistanceJointDef(RigidBody* body1, RigidBody* body2, bool collide = false)
            : JointDef(Joint::Type::DISTANCE, body1, body2, collide) {}
    };

    struct FrictionJointDef : public JointDef, public FrictionJointData {
        FrictionJointDef(RigidBody* body1, RigidBody* body2, bool collide = false)
            : JointDef(Joint::Type::FRICTION, body1, body2, collide) {}
    };

    struct GearJointDef : public JointDef, public GearJointData {
        GearJointDef(RigidBody* body1, RigidBody* body2, bool collide = false)
            : JointDef(Joint::Type::GEAR, body1, body2, collide) {}
    };

    struct MotorJointDef : public JointDef, public MotorJointData {
        MotorJointDef(RigidBody* body1, RigidBody* body2, bool collide = false)
            : JointDef(Joint::Type::MOTOR, body1, body2, collide) {}
    };

    struct MouseJointDef : public JointDef, public MouseJointData {
        MouseJointDef(RigidBody* body1, RigidBody* body2, bool collide = false)
            : JointDef(Joint::Type::MOUSE, body1, body2, collide) {}
    };

    struct PrismaticJointDef : public JointDef, public PrismaticJointData {
        PrismaticJointDef(RigidBody* body1, RigidBody* body2, bool collide = false)
            : JointDef(Joint::Type::PRISMATIC, body1, body2, collide) {}
    };

    struct PulleyJointDef : public JointDef, public PulleyJointData {
        PulleyJointDef(RigidBody* body1, RigidBody* body2, bool collide = true)
            : JointDef(Joint::Type::PULLEY, body1, body2, collide) {}
    };

    struct RevoluteJointDef : public JointDef, public RevoluteJointData {
        RevoluteJointDef(RigidBody* body1, RigidBody* body2, bool collide = false)
            : JointDef(Joint::Type::REVOLUTE, body1, body2, collide) {}
    };

    struct WeldJointDef : public JointDef, public WeldJointData {
        WeldJointDef(RigidBody* body1, RigidBody* body2, bool collide = false)
            : JointDef(Joint::Type::WELD, body1, body2, collide) {}
    };

    struct WheelJointDef : public JointDef, public WheelJointData {
        WheelJointDef(RigidBody* body1, RigidBody* body2, bool collide = false)
            : JointDef(Joint::Type::WHEEL, body1, body2, collide) {}
    };

    // Traits
    template<typename JointDataT> struct JointDefFor;
    template<> struct JointDefFor<DistanceJointData> { using Type = DistanceJointDef; };
    template<> struct JointDefFor<FrictionJointData> { using Type = FrictionJointDef; };
    template<> struct JointDefFor<GearJointData> { using Type = GearJointDef; };
    template<> struct JointDefFor<MotorJointData> { using Type = MotorJointDef; };
    template<> struct JointDefFor<MouseJointData> { using Type = MouseJointDef; };
    template<> struct JointDefFor<PrismaticJointData> { using Type = PrismaticJointDef; };
    template<> struct JointDefFor<PulleyJointData> { using Type = PulleyJointDef; };
    template<> struct JointDefFor<RevoluteJointData> { using Type = RevoluteJointDef; };
    template<> struct JointDefFor<WeldJointData> { using Type = WeldJointDef; };
    template<> struct JointDefFor<WheelJointData> { using Type = WheelJointDef; };
};
