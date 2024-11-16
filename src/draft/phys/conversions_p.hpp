#pragma once

#include "draft/phys/joint_def.hpp"
#include "draft/phys/rigid_body.hpp"
#include "draft/phys/filter.hpp"
#include "draft/phys/shapes/circle_shape.hpp"
#include "draft/phys/shapes/edge_shape.hpp"
#include "draft/phys/shapes/polygon_shape.hpp"

#include "box2d/b2_body.h"
#include "box2d/b2_joint.h"
#include "box2d/b2_distance_joint.h"
#include "box2d/b2_friction_joint.h"
#include "box2d/b2_motor_joint.h"
#include "box2d/b2_mouse_joint.h"
#include "box2d/b2_prismatic_joint.h"
#include "box2d/b2_pulley_joint.h"
#include "box2d/b2_revolute_joint.h"
#include "box2d/b2_weld_joint.h"
#include "box2d/b2_wheel_joint.h"
#include "box2d/b2_gear_joint.h"

namespace Draft {
    class BodyDef;
    
    b2BodyType bodytype_to_b2(const RigidBody::BodyType& type);
    b2BodyDef bodydef_to_b2(const BodyDef& def);

    b2PolygonShape shape_to_b2(const PolygonShape& shape);
    b2CircleShape shape_to_b2(const CircleShape& shape);
    b2EdgeShape shape_to_b2(const EdgeShape& shape);

    b2Filter filter_to_b2(const PhysMask& def);

    b2JointType jointtype_to_b2(const Joint::Type& type);
    void jointdef_base_to_b2(const JointDef& def, b2JointDef& tmp);
    b2DistanceJointDef jointdef_to_b2(const DistanceJointDef& def);
    b2FrictionJointDef jointdef_to_b2(const FrictionJointDef& def);
    b2GearJointDef jointdef_to_b2(const GearJointDef& def);
    b2MotorJointDef jointdef_to_b2(const MotorJointDef& def);
    b2MouseJointDef jointdef_to_b2(const MouseJointDef& def);
    b2PrismaticJointDef jointdef_to_b2(const PrismaticJointDef& def);
    b2PulleyJointDef jointdef_to_b2(const PulleyJointDef& def);
    b2RevoluteJointDef jointdef_to_b2(const RevoluteJointDef& def);
    b2WeldJointDef jointdef_to_b2(const WeldJointDef& def);
    b2WheelJointDef jointdef_to_b2(const WheelJointDef& def);
};