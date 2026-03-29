#include <gtest/gtest.h>
#include "draft/phys/fixture_def.hpp"
#include "draft/phys/shapes/circle_shape.hpp"

TEST(FixtureDef, DefaultConstructor)
{
    Draft::FixtureDef def;
    ASSERT_EQ(def.shape, nullptr);
    ASSERT_EQ(def.friction, 0.2f);
    ASSERT_EQ(def.restitution, 0.f);
    ASSERT_EQ(def.restitutionThreshold, 1.f);
    ASSERT_EQ(def.density, 0.f);
    ASSERT_FALSE(def.isSensor);
    ASSERT_EQ(def.filter.categoryBits, 0x0001);
    ASSERT_EQ(def.filter.maskBits, 0xFFFF);
    ASSERT_EQ(def.filter.groupIndex, 0);
}

TEST(FixtureDef, CustomValues)
{
    Draft::CircleShape circle; // A concrete shape to assign
    Draft::FixtureDef def;
    def.shape = &circle;
    def.friction = 0.5f;
    def.restitution = 0.3f;
    def.restitutionThreshold = 0.5f;
    def.density = 1.5f;
    def.isSensor = true;
    def.filter.categoryBits = 0x0004;
    def.filter.maskBits = 0xFF00;
    def.filter.groupIndex = -1;

    ASSERT_EQ(def.shape, &circle);
    ASSERT_EQ(def.friction, 0.5f);
    ASSERT_EQ(def.restitution, 0.3f);
    ASSERT_EQ(def.restitutionThreshold, 0.5f);
    ASSERT_EQ(def.density, 1.5f);
    ASSERT_TRUE(def.isSensor);
    ASSERT_EQ(def.filter.categoryBits, 0x0004);
    ASSERT_EQ(def.filter.maskBits, 0xFF00);
    ASSERT_EQ(def.filter.groupIndex, -1);
}
