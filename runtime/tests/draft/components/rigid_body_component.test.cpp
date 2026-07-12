#include <gtest/gtest.h>
#include "draft/components/rigid_body_component.hpp"

using namespace Draft;

TEST(NativeBodyComponent, DefaultConstructionIsInvalid)
{
    NativeBodyComponent native;

    EXPECT_EQ(native.bodyPtr, nullptr);
    EXPECT_FALSE(native.is_valid());
    EXPECT_FALSE(static_cast<bool>(native));
    EXPECT_EQ(native.deltaType, BodyType::STATIC);
    EXPECT_TRUE(native.deltaAllowSleep);
    EXPECT_TRUE(native.deltaAwake);
    EXPECT_TRUE(native.deltaEnabled);
    EXPECT_FALSE(native.deltaBullet);
    EXPECT_FALSE(native.deltaFixedRotation);
    EXPECT_FLOAT_EQ(native.deltaGravityScale, 1.f);
}

TEST(RigidBodyComponent, DefaultConstructionMatchesDocumentedDefaults)
{
    RigidBodyComponent body;

    EXPECT_EQ(body.type, BodyType::STATIC);
    EXPECT_FLOAT_EQ(body.linearVelocity.x, 0.f);
    EXPECT_FLOAT_EQ(body.linearVelocity.y, 0.f);
    EXPECT_FLOAT_EQ(body.angularVelocity, 0.f);
    EXPECT_TRUE(body.allowSleep);
    EXPECT_TRUE(body.awake);
    EXPECT_TRUE(body.enabled);
    EXPECT_FALSE(body.bullet);
    EXPECT_FALSE(body.fixedRotation);
    EXPECT_FLOAT_EQ(body.gravityScale, 1.f);
    EXPECT_EQ(body.m_nativeHandlePtr, nullptr);
}

TEST(RigidBodyComponent, AccessorsWithNoNativeHandleReturnSafeDefaults)
{
    RigidBodyComponent body;

    EXPECT_FLOAT_EQ(body.get_world_center().x, 0.f);
    EXPECT_FLOAT_EQ(body.get_mass(), 0.f);
    EXPECT_FLOAT_EQ(body.get_inertia(), 0.f);
    EXPECT_FALSE(body.is_touching(RigidBodyComponent{}));

    // None of these should dereference a null m_nativeHandlePtr
    body.apply_force({1.f, 0.f}, true);
    body.apply_torque(1.f);
    body.apply_linear_impulse({1.f, 0.f});
    body.apply_angular_impulse(1.f);
}
