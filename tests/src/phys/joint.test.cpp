#include <gtest/gtest.h>
#include "draft/phys/joint.hpp"
#include "draft/phys/world.hpp"
#include "draft/phys/rigid_body.hpp"
#include "draft/phys/body_def.hpp"
#include "draft/phys/joint_def.hpp"

// Helper function to create dummy rigid bodies
void create_dummy_bodies_for_joint_test(Draft::World& world, Draft::RigidBody*& bodyA, Draft::RigidBody*& bodyB) {
    Draft::BodyDef defA;
    defA.position = {0.f, 0.f};
    bodyA = world.create_rigid_body(defA);

    Draft::BodyDef defB;
    defB.position = {2.f, 0.f};
    bodyB = world.create_rigid_body(defB);
}

TEST(Joint, TypeEnum)
{
    ASSERT_EQ(static_cast<int>(Draft::Joint::Type::UNKNOWN), 0);
    ASSERT_EQ(static_cast<int>(Draft::Joint::Type::DISTANCE), 1);
    ASSERT_EQ(static_cast<int>(Draft::Joint::Type::REVOLUTE), 2);
    ASSERT_EQ(static_cast<int>(Draft::Joint::Type::PRISMATIC), 3);
    ASSERT_EQ(static_cast<int>(Draft::Joint::Type::PULLEY), 4);
    ASSERT_EQ(static_cast<int>(Draft::Joint::Type::GEAR), 5);
    ASSERT_EQ(static_cast<int>(Draft::Joint::Type::MOUSE), 6);
    ASSERT_EQ(static_cast<int>(Draft::Joint::Type::WHEEL), 7);
    ASSERT_EQ(static_cast<int>(Draft::Joint::Type::WELD), 8);
    ASSERT_EQ(static_cast<int>(Draft::Joint::Type::ROPE), 9);
    ASSERT_EQ(static_cast<int>(Draft::Joint::Type::FRICTION), 10);
    ASSERT_EQ(static_cast<int>(Draft::Joint::Type::MOTOR), 11);
}

TEST(Joint, CreateDistanceJointThroughWorldAndBaseProperties)
{
    Draft::World world({0.f, -9.8f});
    Draft::RigidBody *body1, *body2;
    create_dummy_bodies_for_joint_test(world, body1, body2);

    Draft::DistanceJointDef jointDef(body1, body2, true); // collideConnected = true
    jointDef.anchorA = {0.f, 0.f};
    jointDef.anchorB = {0.f, 0.f};
    jointDef.length = 2.f;

    Draft::Joint* joint = world.create_joint(jointDef);
    ASSERT_NE(joint, nullptr);
    ASSERT_EQ(joint->get_type(), Draft::Joint::Type::DISTANCE);
    ASSERT_EQ(joint->get_world(), &world);
    ASSERT_EQ(joint->get_body_a(), body1);
    ASSERT_EQ(joint->get_body_b(), body2);
    ASSERT_TRUE(joint->get_collide_connected());
    ASSERT_TRUE(joint->is_enabled()); // Joints are enabled by default

    world.destroy_joint(joint);
    world.destroy_body(body1);
    world.destroy_body(body2);
}

TEST(DistanceJoint, SetGetLength)
{
    Draft::World world({0.f, 0.f});
    Draft::RigidBody *body1, *body2;
    create_dummy_bodies_for_joint_test(world, body1, body2);

    Draft::DistanceJointDef jointDef(body1, body2);
    jointDef.length = 1.0f;
    Draft::DistanceJoint* joint = static_cast<Draft::DistanceJoint*>(world.create_joint(jointDef));
    ASSERT_NE(joint, nullptr);

    ASSERT_EQ(joint->get_length(), 1.0f);
    joint->set_length(5.0f);
    ASSERT_EQ(joint->get_length(), 5.0f);

    world.destroy_joint(joint);
    world.destroy_body(body1);
    world.destroy_body(body2);
}

TEST(DistanceJoint, SetGetMinMaxLength)
{
    Draft::World world({0.f, 0.f});
    Draft::RigidBody *body1, *body2;
    create_dummy_bodies_for_joint_test(world, body1, body2);

    Draft::DistanceJointDef jointDef(body1, body2);
    jointDef.minLength = 0.5f;
    jointDef.maxLength = 10.0f;
    Draft::DistanceJoint* joint = static_cast<Draft::DistanceJoint*>(world.create_joint(jointDef));
    ASSERT_NE(joint, nullptr);

    ASSERT_EQ(joint->get_min_length(), 0.5f);
    ASSERT_EQ(joint->get_max_length(), 10.0f);

    joint->set_min_length(0.1f);
    joint->set_max_length(15.0f);
    ASSERT_EQ(joint->get_min_length(), 0.1f);
    ASSERT_EQ(joint->get_max_length(), 15.0f);

    world.destroy_joint(joint);
    world.destroy_body(body1);
    world.destroy_body(body2);
}

TEST(DistanceJoint, SetGetStiffnessDamping)
{
    Draft::World world({0.f, 0.f});
    Draft::RigidBody *body1, *body2;
    create_dummy_bodies_for_joint_test(world, body1, body2);

    Draft::DistanceJointDef jointDef(body1, body2);
    jointDef.stiffness = 0.7f;
    jointDef.damping = 0.2f;
    Draft::DistanceJoint* joint = static_cast<Draft::DistanceJoint*>(world.create_joint(jointDef));
    ASSERT_NE(joint, nullptr);

    ASSERT_EQ(joint->get_stiffness(), 0.7f);
    ASSERT_EQ(joint->get_damping(), 0.2f);

    joint->set_stiffness(0.9f);
    joint->set_damping(0.4f);
    ASSERT_EQ(joint->get_stiffness(), 0.9f);
    ASSERT_EQ(joint->get_damping(), 0.4f);

    world.destroy_joint(joint);
    world.destroy_body(body1);
    world.destroy_body(body2);
}

TEST(FrictionJoint, SetGetMaxForceMaxTorque)
{
    Draft::World world({0.f, 0.f});
    Draft::RigidBody *body1, *body2;
    create_dummy_bodies_for_joint_test(world, body1, body2);

    Draft::FrictionJointDef jointDef(body1, body2);
    jointDef.maxForce = 10.0f;
    jointDef.maxTorque = 5.0f;
    Draft::FrictionJoint* joint = static_cast<Draft::FrictionJoint*>(world.create_joint(jointDef));
    ASSERT_NE(joint, nullptr);

    ASSERT_EQ(joint->get_max_force(), 10.0f);
    ASSERT_EQ(joint->get_max_torque(), 5.0f);

    joint->set_max_force(20.0f);
    joint->set_max_torque(10.0f);
    ASSERT_EQ(joint->get_max_force(), 20.0f);
    ASSERT_EQ(joint->get_max_torque(), 10.0f);

    world.destroy_joint(joint);
    world.destroy_body(body1);
    world.destroy_body(body2);
}
