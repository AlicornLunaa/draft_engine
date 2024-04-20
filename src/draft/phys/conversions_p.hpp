#pragma once

#include "draft/phys/rigid_body.hpp"
#include "draft/phys/filter.hpp"
#include "box2d/b2_body.h"

namespace Draft {
    class BodyDef;
    
    b2BodyType bodytype_to_b2(const RigidBody::BodyType& type);
    b2BodyDef bodydef_to_b2(const BodyDef& def);

    b2PolygonShape shape_to_b2(const PolygonShape& shape);
    b2CircleShape shape_to_b2(const CircleShape& shape);
    b2EdgeShape shape_to_b2(const EdgeShape& shape);

    b2Filter filter_to_b2(const Filter& def);
};