#include <gtest/gtest.h>
#include "draft/phys/joint_def.hpp"
#include "draft/phys/world.hpp"
#include "draft/phys/rigid_body.hpp"
#include "draft/phys/body_def.hpp"

// Helper function to create dummy rigid bodies
void create_dummy_bodies(Draft::World& world, Draft::RigidBody*& bodyA, Draft::RigidBody*& bodyB) {
    Draft::BodyDef defA;
    defA.position = {0.f, 0.f};
    bodyA = world.create_rigid_body(defA);

    Draft::BodyDef defB;
    defB.position = {1.f, 1.f};
    bodyB = world.create_rigid_body(defB);
}

TEST(GenericJointData, DefaultConstructor)
{
    Draft::GenericJointData data;
    ASSERT_FALSE(data.collideConnected);
}

TEST(JointDef, Constructor)
{
    Draft::World world({0.f, 0.f});
    Draft::RigidBody *bodyA, *bodyB;
    create_dummy_bodies(world, bodyA, bodyB);

    Draft::JointDef def(Draft::Joint::Type::DISTANCE, bodyA, bodyB, true);
    ASSERT_EQ(def.type, Draft::Joint::Type::DISTANCE);
    ASSERT_EQ(def.bodyA, bodyA);
    ASSERT_EQ(def.bodyB, bodyB);
    ASSERT_TRUE(def.collideConnected);

    world.destroy_body(bodyA);
    world.destroy_body(bodyB);
}

TEST(DistanceJointDef, DefaultConstructor)
{
    Draft::World world({0.f, 0.f});
    Draft::RigidBody *bodyA, *bodyB;
    create_dummy_bodies(world, bodyA, bodyB);

    Draft::DistanceJointDef def(bodyA, bodyB);
    ASSERT_EQ(def.type, Draft::Joint::Type::DISTANCE);
    ASSERT_EQ(def.bodyA, bodyA);
    ASSERT_EQ(def.bodyB, bodyB);
    ASSERT_FALSE(def.collideConnected);
    ASSERT_EQ(def.anchorA.x, 0.f);
    ASSERT_EQ(def.anchorA.y, 0.f);
    ASSERT_EQ(def.anchorB.x, 0.f);
    ASSERT_EQ(def.anchorB.y, 0.f);
    ASSERT_EQ(def.length, 1.f);
    ASSERT_EQ(def.minLength, 0.f);
    ASSERT_EQ(def.maxLength, FLT_MAX);
    ASSERT_EQ(def.stiffness, 0.f);
    ASSERT_EQ(def.damping, 0.f);

    world.destroy_body(bodyA);
    world.destroy_body(bodyB);
}

TEST(FrictionJointDef, DefaultConstructor)
{
    Draft::World world({0.f, 0.f});
    Draft::RigidBody *bodyA, *bodyB;
    create_dummy_bodies(world, bodyA, bodyB);

    Draft::FrictionJointDef def(bodyA, bodyB);
    ASSERT_EQ(def.type, Draft::Joint::Type::FRICTION);
    ASSERT_EQ(def.bodyA, bodyA);
    ASSERT_EQ(def.bodyB, bodyB);
    ASSERT_FALSE(def.collideConnected);
    ASSERT_EQ(def.anchorA.x, 0.f);
    ASSERT_EQ(def.anchorA.y, 0.f);
    ASSERT_EQ(def.anchorB.x, 0.f);
    ASSERT_EQ(def.anchorB.y, 0.f);
    ASSERT_EQ(def.maxForce, 0.f);
    ASSERT_EQ(def.maxTorque, 0.f);

    world.destroy_body(bodyA);
    world.destroy_body(bodyB);
}
