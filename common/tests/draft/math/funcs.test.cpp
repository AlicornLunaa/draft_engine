#include <gtest/gtest.h>
#include "draft/math/funcs.hpp"
#include "draft/math/glm.hpp"

TEST(Funcs, ModFloor)
{
    ASSERT_EQ(glm::mod_floor(5, 3), 2);
    ASSERT_EQ(glm::mod_floor(-5, 3), 1);
}

TEST(Funcs, DivFloor)
{
    ASSERT_EQ(glm::div_floor(5.f, 3.f), 1.f);
    ASSERT_EQ(glm::div_floor(-5.f, 3.f), -2.f);
}

TEST(Funcs, Cross)
{
    Draft::Vector2f v1(1.f, 2.f);
    Draft::Vector2f v2(3.f, 4.f);
    ASSERT_EQ(glm::cross(v1, v2), -2.f);
}

TEST(Funcs, Lerp)
{
    ASSERT_EQ(glm::lerp(0.f, 10.f, 0.5f), 5.f);
}

TEST(Funcs, Perpendicular)
{
    Draft::Vector2f v(1.f, 2.f);
    Draft::Vector2f p = glm::perpendicular(v);
    ASSERT_EQ(p.x, -2.f);
    ASSERT_EQ(p.y, 1.f);
}

TEST(Funcs, Map)
{
    ASSERT_EQ(glm::map(5.f, 0.f, 10.f, 0.f, 100.f), 50.f);
    ASSERT_EQ(glm::map(0.f, 0.f, 10.f, -1.f, 1.f), -1.f);
    ASSERT_EQ(glm::map(10.f, 0.f, 10.f, -1.f, 1.f), 1.f);
}

TEST(Funcs, MapExtrapolatesPastTheInputRangeInsteadOfClamping)
{
    ASSERT_EQ(glm::map(15.f, 0.f, 10.f, 0.f, 100.f), 150.f);
    ASSERT_EQ(glm::map(-5.f, 0.f, 10.f, 0.f, 100.f), -50.f);
}

TEST(Funcs, InverseLerp)
{
    ASSERT_EQ(glm::inverse_lerp(5.f, 0.f, 10.f), 0.5f);
    ASSERT_EQ(glm::inverse_lerp(0.f, 0.f, 10.f), 0.f);
    ASSERT_EQ(glm::inverse_lerp(10.f, 0.f, 10.f), 1.f);
}

TEST(Funcs, MapAndInverseLerpAreInverses)
{
    float mapped = glm::map(3.f, 0.f, 10.f, 20.f, 40.f);
    ASSERT_EQ(glm::inverse_lerp(mapped, 20.f, 40.f), glm::inverse_lerp(3.f, 0.f, 10.f));
}
