#include "box2d/b2_fixture.h"
#include "draft/phys/conversions_p.hpp"
#include "draft/phys/body_def.hpp"
#include "draft/math/vector2_p.hpp"

#include "box2d/b2_circle_shape.h"
#include "box2d/b2_edge_shape.h"
#include "box2d/b2_polygon_shape.h"

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

    b2Filter filter_to_b2(const Filter& filter){
        b2Filter f;
        f.categoryBits = filter.categoryBits;
        f.maskBits = filter.maskBits;
        f.groupIndex = filter.groupIndex;
        return f;
    }
};