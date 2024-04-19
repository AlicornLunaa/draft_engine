#pragma once

#include "draft/phys/rigid_body.hpp"
#include "box2d/b2_body.h"

namespace Draft {
    class BodyDef;
    b2BodyType bodytype_to_b2(const RigidBody::BodyType& type);
    b2BodyDef bodydef_to_b2(const BodyDef& def);
};