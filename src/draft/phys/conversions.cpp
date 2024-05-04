#include "box2d/b2_body.h"
#include "draft/phys/conversions_p.hpp"
#include "draft/phys/rigid_body.hpp"
#include "draft/phys/body_def.hpp"
#include "draft/math/vector2_p.hpp"

#include "box2d/b2_circle_shape.h"
#include "box2d/b2_edge_shape.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_distance_joint.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_joint.h"
#include "box2d/b2_friction_joint.h"
#include "box2d/b2_gear_joint.h"

namespace Draft {
    b2BodyType bodytype_to_b2(const RigidBody::BodyType& type){
        switch(type){
            case RigidBody::DYNAMIC:
                return b2_dynamicBody;

            case RigidBody::KINEMATIC:
                return b2_kinematicBody;

            case RigidBody::STATIC:
            default:
                return b2_staticBody;
        }
    }

    b2BodyDef bodydef_to_b2(const BodyDef& def){
        b2BodyDef tmp{};
        tmp.type = bodytype_to_b2(def.type);
        tmp.position = vector_to_b2(def.position);
        tmp.angle = def.angle;
        tmp.linearVelocity = vector_to_b2(def.linearVelocity);
        tmp.angularVelocity = def.angularVelocity;
        tmp.linearDamping = def.linearDamping;
        tmp.angularDamping = def.angularDamping;
        tmp.allowSleep = def.allowSleep;
        tmp.awake = def.awake;
        tmp.fixedRotation = def.fixedRotation;
        tmp.bullet = def.bullet;
        tmp.enabled = def.enabled;
        tmp.gravityScale = def.gravityScale;
        return tmp;
    }

    b2PolygonShape shape_to_b2(const PolygonShape& shape){
        // Convert verticees to new point
        size_t count = shape.get_vertex_count();
        b2Vec2 physVerts[count];

        for(size_t i = 0; i < count; i++){
            physVerts[i] = vector_to_b2(shape.get_vertex(i));
        }

        // Create shape;
        b2PolygonShape s;
        s.Set(&physVerts[0], count);
        return s;
    }

    b2CircleShape shape_to_b2(const CircleShape& shape){
        b2CircleShape s;
        s.m_p.Set(shape.position.x, shape.position.y);
        s.m_radius = shape.get_radius();
        return s;
    }

    b2EdgeShape shape_to_b2(const EdgeShape& shape){
        b2EdgeShape s;
        s.SetTwoSided(vector_to_b2(shape.get_start()), vector_to_b2(shape.get_end()));
        return s;
    }

    b2Filter filter_to_b2(const PhysMask& filter){
        b2Filter f;
        f.categoryBits = filter.categoryBits;
        f.maskBits = filter.maskBits;
        f.groupIndex = filter.groupIndex;
        return f;
    }

    b2JointType jointtype_to_b2(const Joint::Type& type){
        switch(type){
            case Joint::Type::DISTANCE:
                return e_distanceJoint;

            case Joint::Type::REVOLUTE:
                return e_revoluteJoint;

            case Joint::Type::PRISMATIC:
                return e_prismaticJoint;

            case Joint::Type::PULLEY:
                return e_pulleyJoint;

            case Joint::Type::GEAR:
                return e_gearJoint;

            case Joint::Type::MOUSE:
                return e_mouseJoint;

            case Joint::Type::WHEEL:
                return e_wheelJoint;

            case Joint::Type::WELD:
                return e_weldJoint;

            case Joint::Type::ROPE:
                return e_ropeJoint;

            case Joint::Type::FRICTION:
                return e_frictionJoint;

            case Joint::Type::MOTOR:
                return e_motorJoint;

            case Joint::Type::UNKNOWN:
            default:
                return e_unknownJoint;
        }
    }

    void jointdef_base_to_b2(const JointDef& def, b2JointDef& tmp){
        tmp.type = jointtype_to_b2(def.type);
        tmp.bodyA = (b2Body*)def.bodyA->get_body_ptr();
        tmp.bodyB = (b2Body*)def.bodyB->get_body_ptr();
        tmp.collideConnected = def.collideConnected;
    }

    b2DistanceJointDef jointdef_to_b2(const DistanceJointDef& def){
        b2DistanceJointDef tmp{};
        jointdef_base_to_b2(def, tmp);
        tmp.localAnchorA = vector_to_b2(def.anchorA);
        tmp.localAnchorB = vector_to_b2(def.anchorB);
        tmp.length = def.length;
        tmp.minLength = def.minLength;
        tmp.maxLength = def.maxLength;
        tmp.stiffness = def.stiffness;
        tmp.damping = def.damping;
        return tmp;
    }

    b2FrictionJointDef jointdef_to_b2(const FrictionJointDef& def){
        b2FrictionJointDef tmp{};
        jointdef_base_to_b2(def, tmp);
        tmp.localAnchorA = vector_to_b2(def.anchorA);
        tmp.localAnchorB = vector_to_b2(def.anchorB);
        tmp.maxForce = def.maxForce;
        tmp.maxTorque = def.maxTorque;
        return tmp;
    }

    b2GearJointDef jointdef_to_b2(const GearJointDef& def){
        b2GearJointDef tmp{};
        jointdef_base_to_b2(def, tmp);
        tmp.ratio = def.ratio;
        tmp.joint1 = (b2Joint*)def.joint1->get_joint_ptr();
        tmp.joint2 = (b2Joint*)def.joint2->get_joint_ptr();
        return tmp;
    }

    b2MotorJointDef jointdef_to_b2(const MotorJointDef& def){
        b2MotorJointDef tmp{};
        jointdef_base_to_b2(def, tmp);
        tmp.linearOffset = vector_to_b2(def.linearOffset);
        tmp.angularOffset = def.angularOffset;
        tmp.maxForce = def.maxForce;
        tmp.maxTorque = def.maxTorque;
        tmp.correctionFactor = def.correctionFactor;
        return tmp;
    }

    b2MouseJointDef jointdef_to_b2(const MouseJointDef& def){
        b2MouseJointDef tmp{};
        jointdef_base_to_b2(def, tmp);
        tmp.target = vector_to_b2(def.target);
        tmp.maxForce = def.maxForce;
        tmp.stiffness = def.stiffness;
        tmp.damping = def.damping;
        return tmp;
    }

    b2PrismaticJointDef jointdef_to_b2(const PrismaticJointDef& def){
        b2PrismaticJointDef tmp{};
        jointdef_base_to_b2(def, tmp);
        tmp.localAnchorA = vector_to_b2(def.anchorA);
        tmp.localAnchorB = vector_to_b2(def.anchorB);
        tmp.localAxisA = vector_to_b2(def.localAxisA);
        tmp.referenceAngle = def.referenceAngle;
        tmp.enableLimit = def.enableLimit;
        tmp.lowerTranslation = def.lowerTranslation;
        tmp.upperTranslation = def.upperTranslation;
        tmp.enableMotor = def.enableMotor;
        tmp.maxMotorForce = def.maxMotorForce;
        tmp.motorSpeed = def.motorSpeed;
        return tmp;
    }

    b2PulleyJointDef jointdef_to_b2(const PulleyJointDef& def){
        b2PulleyJointDef tmp{};
        jointdef_base_to_b2(def, tmp);
        tmp.groundAnchorA = vector_to_b2(def.groundAnchorA);
        tmp.groundAnchorB = vector_to_b2(def.groundAnchorB);
        tmp.localAnchorA = vector_to_b2(def.localAnchorA);
        tmp.localAnchorB = vector_to_b2(def.localAnchorB);
        tmp.lengthA = def.lengthA;
        tmp.lengthB = def.lengthB;
        tmp.ratio = def.ratio;
        return tmp;
    }

    b2RevoluteJointDef jointdef_to_b2(const RevoluteJointDef& def){
        b2RevoluteJointDef tmp{};
        jointdef_base_to_b2(def, tmp);
        tmp.localAnchorA = vector_to_b2(def.localAnchorA);
        tmp.localAnchorB = vector_to_b2(def.localAnchorB);
        tmp.referenceAngle = def.referenceAngle;
        tmp.lowerAngle = def.lowerAngle;
        tmp.upperAngle = def.upperAngle;
        tmp.maxMotorTorque = def.maxMotorTorque;
        tmp.motorSpeed = def.motorSpeed;
        tmp.enableLimit = def.enableLimit;
        tmp.enableMotor = def.enableMotor;
        return tmp;
    }

    b2WeldJointDef jointdef_to_b2(const WeldJointDef& def){
        b2WeldJointDef tmp{};
        jointdef_base_to_b2(def, tmp);
        tmp.localAnchorA = vector_to_b2(def.anchorA);
        tmp.localAnchorB = vector_to_b2(def.anchorB);
        tmp.referenceAngle = def.referenceAngle;
        tmp.stiffness = def.stiffness;
        tmp.damping = def.damping;
        return tmp;
    }

    b2WheelJointDef jointdef_to_b2(const WheelJointDef& def){
        b2WheelJointDef tmp{};
        jointdef_base_to_b2(def, tmp);
        tmp.localAnchorA = vector_to_b2(def.anchorA);
        tmp.localAnchorB = vector_to_b2(def.anchorB);
        tmp.localAxisA = vector_to_b2(def.localAxis);
        tmp.lowerTranslation = def.lowerTranslation;
        tmp.upperTranslation = def.upperTranslation;
        tmp.maxMotorTorque = def.maxMotorTorque;
        tmp.motorSpeed = def.motorSpeed;
        tmp.stiffness = def.stiffness;
        tmp.damping = def.damping;
        tmp.enableLimit = def.enableLimit;
        tmp.enableMotor = def.enableMotor;
        return tmp;
    }
};