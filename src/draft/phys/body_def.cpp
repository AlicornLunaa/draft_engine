#include "draft/phys/body_def_p.hpp"
#include "draft/phys/body_def.hpp"
#include "draft/math/vector2_p.hpp"

namespace Draft {
    // Conversion functions
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
};