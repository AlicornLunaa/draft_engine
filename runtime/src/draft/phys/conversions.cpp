#include "draft/phys/body_def.hpp"
#include "draft/phys/conversions_p.hpp"
#include "draft/phys/rigid_body.hpp"
#include "draft/phys/vector2_p.hpp"
#include "draft/util/logger.hpp"

#include <array>
#include <stdexcept>
#include <string>

namespace Draft {
    b2PolygonShape shape_to_b2(const PolygonShape& shape){
        size_t count = shape.get_vertex_count();
        if(count < 3 || count > b2_maxPolygonVertices){
            throw std::runtime_error("PolygonShape must have between 3 and " + std::to_string(b2_maxPolygonVertices) + " vertices");
        }

        std::array<b2Vec2, b2_maxPolygonVertices> physVerts{};
        for(size_t i = 0; i < count; i++){
            physVerts[i] = vector_to_b2(shape.get_vertex(i));
        }

        b2PolygonShape s;
        s.Set(physVerts.data(), count);
        return s;
    }

    b2CircleShape shape_to_b2(const CircleShape& shape){
        const Vector2f& position = shape.get_position();
        b2CircleShape s;
        s.m_p.Set(position.x, position.y);
        s.m_radius = shape.get_radius();
        return s;
    }

    b2EdgeShape shape_to_b2(const EdgeShape& shape){
        b2EdgeShape s;
        s.SetTwoSided(vector_to_b2(shape.get_start()), vector_to_b2(shape.get_end()));
        return s;
    }

    b2ChainShape shape_to_b2(const ChainShape& shape){
        std::vector<b2Vec2> rawPoints;
        rawPoints.resize(shape.get_points().size());

        for(size_t i = 0; i < shape.get_points().size(); i++){
            rawPoints[i] = vector_to_b2(shape.get_points()[i]);
        }

        auto type = shape.get_chain_type();
        b2ChainShape s;

        if(type == ChainShape::CHAIN){
            s.CreateChain(rawPoints.data(), shape.get_points().size(), vector_to_b2(shape.get_previous()), vector_to_b2(shape.get_next()));
        } else if(type == ChainShape::LOOP){
            s.CreateLoop(rawPoints.data(), rawPoints.size());
        } else {
            Logger::println(LogLevel::Warning, "Chain Shape", "Invalid chain type");
        }

        return s;
    }

    b2BodyType bodytype_to_b2(const BodyType& type){
        switch(type){
            case BodyType::DYNAMIC:
                return b2_dynamicBody;

            case BodyType::KINEMATIC:
                return b2_kinematicBody;

            case BodyType::STATIC:
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
        if(!def.joint1 || !def.joint2){
            throw std::invalid_argument("GearJointDef requires both joint1 and joint2 to be set");
        }

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

    b2Transform transform_to_b2(const Vector2f& pos, float rot){
        b2Transform tmp{};
        tmp.Set(vector_to_b2(pos), rot);
        return tmp;
    }

    b2RayCastInput raycast_to_b2(const RaycastProps& props){
        b2RayCastInput tmp{};
        tmp.p1 = vector_to_b2(props.origin);
        tmp.p2 = vector_to_b2(props.translation);
        tmp.maxFraction = props.maxFraction;
        return tmp;
    }

    RaycastResult b2_to_raycast_result(const b2RayCastOutput& props){
        RaycastResult tmp{};
        tmp.normal = b2_to_vector<float>(props.normal);
        tmp.fraction = props.fraction;
        return tmp;
    }
};
