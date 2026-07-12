#include <gtest/gtest.h>
#include "draft/math/rect.hpp"

TEST(Rect, Constructor)
{
    Draft::FloatRect r1(1.f, 2.f, 3.f, 4.f);
    ASSERT_EQ(r1.x, 1.f);
    ASSERT_EQ(r1.y, 2.f);
    ASSERT_EQ(r1.width, 3.f);
    ASSERT_EQ(r1.height, 4.f);

    Draft::FloatRect r2({1.f, 2.f}, {3.f, 4.f});
    ASSERT_EQ(r2.x, 1.f);
    ASSERT_EQ(r2.y, 2.f);
    ASSERT_EQ(r2.width, 3.f);
    ASSERT_EQ(r2.height, 4.f);
}

TEST(Rect, BoundsConversion)
{
    Draft::FloatRect r1(1.f, 2.f, 3.f, 4.f);
    Draft::Bounds b = r1;

    ASSERT_EQ(b[0].x, 1.f);
    ASSERT_EQ(b[0].y, 2.f);

    ASSERT_EQ(b[1].x, 4.f);
    ASSERT_EQ(b[1].y, 2.f);

    ASSERT_EQ(b[2].x, 4.f);
    ASSERT_EQ(b[2].y, 6.f);

    ASSERT_EQ(b[3].x, 1.f);
    ASSERT_EQ(b[3].y, 6.f);
}

TEST(Rect, ScalarTypeConversion)
{
    Draft::FloatRect r1(1.f, 2.f, 3.f, 4.f);
    Draft::IntRect r2 = r1;

    ASSERT_EQ(r2.x, 1);
    ASSERT_EQ(r2.y, 2);
    ASSERT_EQ(r2.width, 3);
    ASSERT_EQ(r2.height, 4);
}

TEST(Rect, NormalizeCoordinatesCenterIsOrigin)
{
    Draft::FloatRect r(0.f, 0.f, 100.f, 50.f);
    Draft::Vector2f center = glm::normalize_coordinates(r, Draft::Vector2f(50.f, 25.f));

    EXPECT_FLOAT_EQ(center.x, 0.f);
    EXPECT_FLOAT_EQ(center.y, 0.f);
}

TEST(Rect, NormalizeCoordinatesCorners)
{
    Draft::FloatRect r(0.f, 0.f, 100.f, 50.f);

    // Top-left of the rect (y=0) maps to +1 on the normalized y axis (flipped, +y is up).
    Draft::Vector2f topLeft = glm::normalize_coordinates(r, Draft::Vector2f(0.f, 0.f));
    EXPECT_FLOAT_EQ(topLeft.x, -1.f);
    EXPECT_FLOAT_EQ(topLeft.y, 1.f);

    // Bottom-right of the rect maps to (+1, -1).
    Draft::Vector2f bottomRight = glm::normalize_coordinates(r, Draft::Vector2f(100.f, 50.f));
    EXPECT_FLOAT_EQ(bottomRight.x, 1.f);
    EXPECT_FLOAT_EQ(bottomRight.y, -1.f);
}
