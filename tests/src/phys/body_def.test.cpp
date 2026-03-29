#include <gtest/gtest.h>
#include "draft/phys/body_def.hpp"
#include "draft/phys/rigid_body.hpp"

TEST(BodyDef, DefaultConstructor)
{
    Draft::BodyDef def;
    ASSERT_EQ(def.type, Draft::RigidBody::BodyType::STATIC);
    ASSERT_EQ(def.position.x, 0.f);
    ASSERT_EQ(def.position.y, 0.f);
    ASSERT_EQ(def.angle, 0.f);
    ASSERT_EQ(def.linearVelocity.x, 0.f);
    ASSERT_EQ(def.linearVelocity.y, 0.f);
    ASSERT_EQ(def.angularVelocity, 0.f);
    ASSERT_EQ(def.linearDamping, 0.f);
    ASSERT_EQ(def.angularDamping, 0.f);
    ASSERT_TRUE(def.allowSleep);
    ASSERT_TRUE(def.awake);
    ASSERT_FALSE(def.fixedRotation);
    ASSERT_FALSE(def.bullet);
    ASSERT_TRUE(def.enabled);
    ASSERT_EQ(def.gravityScale, 1.f);
}

TEST(BodyDef, CustomValues)
{
    Draft::BodyDef def;
    def.type = Draft::RigidBody::BodyType::DYNAMIC;
    def.position = {10.f, 5.f};
    def.angle = 1.23f;
    def.linearVelocity = {2.f, 3.f};
    def.angularVelocity = 0.5f;
    def.linearDamping = 0.1f;
    def.angularDamping = 0.2f;
    def.allowSleep = false;
    def.awake = false;
    def.fixedRotation = true;
    def.bullet = true;
    def.enabled = false;
    def.gravityScale = 0.5f;

    ASSERT_EQ(def.type, Draft::RigidBody::BodyType::DYNAMIC);
    ASSERT_EQ(def.position.x, 10.f);
    ASSERT_EQ(def.position.y, 5.f);
    ASSERT_EQ(def.angle, 1.23f);
    ASSERT_EQ(def.linearVelocity.x, 2.f);
    ASSERT_EQ(def.linearVelocity.y, 3.f);
    ASSERT_EQ(def.angularVelocity, 0.5f);
    ASSERT_EQ(def.linearDamping, 0.1f);
    ASSERT_EQ(def.angularDamping, 0.2f);
    ASSERT_FALSE(def.allowSleep);
    ASSERT_FALSE(def.awake);
    ASSERT_TRUE(def.fixedRotation);
    ASSERT_TRUE(def.bullet);
    ASSERT_FALSE(def.enabled);
    ASSERT_EQ(def.gravityScale, 0.5f);
}
