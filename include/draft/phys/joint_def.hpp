#pragma once

#include "draft/math/glm.hpp"
#include "draft/phys/joint.hpp"

#include <cfloat>

namespace Draft {
    // Forward declarations
    class RigidBody;
    class Joint;

    // Structs
    struct JointDef {
        // Variables
        Joint::Type type = Joint::Type::UNKNOWN;
        RigidBody* bodyA = nullptr;
        RigidBody* bodyB = nullptr;
        bool collideConnected = true;

        // Constructor
        JointDef(Joint::Type type, RigidBody* body1, RigidBody* body2, bool collide = false){
            this->type = type;
            bodyA = body1;
            bodyB = body2;
            collideConnected = collide;
        }
    };

    struct DistanceJointDef : public JointDef {
        // Variables
        Vector2f anchorA{};
        Vector2f anchorB{};
        float length = 1.f;
        float minLength = 0.f;
        float maxLength = FLT_MAX;
        float stiffness = 0.f;
        float damping = 0.f;

        // Constructor
        DistanceJointDef(RigidBody* body1, RigidBody* body2, bool collide = false) : JointDef(Joint::Type::DISTANCE, body1, body2, collide) {}
    };

    struct FrictionJointDef : public JointDef {
        // Variables
        Vector2f anchorA{};
        Vector2f anchorB{};
        float maxForce = 0.f;
        float maxTorque = 0.f;

        // Constructor
        FrictionJointDef(RigidBody* body1, RigidBody* body2, bool collide = false) : JointDef(Joint::Type::FRICTION, body1, body2, collide) {}
    };

    struct GearJointDef : public JointDef {
        // Variables
        Joint* joint1 = nullptr;
        Joint* joint2 = nullptr;
        float ratio = 1.f;

        // Constructor
        GearJointDef(RigidBody* body1, RigidBody* body2, bool collide = false) : JointDef(Joint::Type::GEAR, body1, body2, collide) {}
    };

    struct MotorJointDef : public JointDef {
        // Variables
        Vector2f linearOffset{};
        float angularOffset = 0.f;
        float maxForce = 1.f;
        float maxTorque = 1.f;
        float correctionFactor = 0.3f;

        // Constructor
        MotorJointDef(RigidBody* body1, RigidBody* body2, bool collide = false) : JointDef(Joint::Type::MOTOR, body1, body2, collide) {}
    };

    struct MouseJointDef : public JointDef {
        // Variables
        Vector2f target{};
        float maxForce = 0.f;
        float stiffness = 0.f;
        float damping = 0.f;

        // Constructor
        MouseJointDef(RigidBody* body1, RigidBody* body2, bool collide = false) : JointDef(Joint::Type::MOUSE, body1, body2, collide) {}
    };

    struct PrismaticJointDef : public JointDef {
        // Variables
        Vector2f anchorA{};
        Vector2f anchorB{};
        Vector2f localAxisA{1, 0};
        float referenceAngle = 0.f;
        bool enableLimit = false;
        float lowerTranslation = 0.f;
        float upperTranslation = 0.f;
        bool enableMotor = false;
        float maxMotorForce = 0.f;
        float motorSpeed = 0.f;

        // Constructor
        PrismaticJointDef(RigidBody* body1, RigidBody* body2, bool collide = false) : JointDef(Joint::Type::PRISMATIC, body1, body2, collide) {}
    };

    struct PulleyJointDef : public JointDef {
        // Variables
        Vector2f groundAnchorA{};
        Vector2f groundAnchorB{};
        Vector2f localAnchorA{};
        Vector2f localAnchorB{};
        float lengthA = 0.f;
        float lengthB = 0.f;
        float ratio = 1.f;

        // Constructor
        PulleyJointDef(RigidBody* body1, RigidBody* body2, bool collide = true) : JointDef(Joint::Type::PULLEY, body1, body2, collide) {}
    };

    struct RevoluteJointDef : public JointDef {
        // Variables
        Vector2f localAnchorA{};
        Vector2f localAnchorB{};
        float referenceAngle = 0.f;
        float lowerAngle = 0.f;
        float upperAngle = 0.f;
        float maxMotorTorque = 0.f;
        float motorSpeed = 0.f;
        bool enableLimit = false;
        bool enableMotor = false;

        // Constructor
        RevoluteJointDef(RigidBody* body1, RigidBody* body2, bool collide = false) : JointDef(Joint::Type::REVOLUTE, body1, body2, collide) {}
    };

    struct WeldJointDef : public JointDef {
        // Variables
        Vector2f anchorA{};
        Vector2f anchorB{};
        float referenceAngle = 0.f;
        float stiffness = 0.f;
        float damping = 0.f;

        // Constructor
        WeldJointDef(RigidBody* body1, RigidBody* body2, bool collide = false) : JointDef(Joint::Type::WELD, body1, body2, collide) {}
    };

    struct WheelJointDef : public JointDef {
        // Variables
        Vector2f anchorA{};
        Vector2f anchorB{};
        Vector2f localAxis{1, 0};
        float lowerTranslation = 0.f;
        float upperTranslation = 0.f;
        float maxMotorTorque = 0.f;
        float motorSpeed = 0.f;
        float stiffness = 0.f;
        float damping = 0.f;
        bool enableLimit = false;
        bool enableMotor = false;

        // Constructor
        WheelJointDef(RigidBody* body1, RigidBody* body2, bool collide = false) : JointDef(Joint::Type::WHEEL, body1, body2, collide) {}
    };
};