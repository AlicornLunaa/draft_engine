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

TEST(Rect, Conversion)
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

    Draft::IntRect r2 = r1;
    ASSERT_EQ(r2.x, 1);
    ASSERT_EQ(r2.y, 2);
    ASSERT_EQ(r2.width, 3);
    ASSERT_EQ(r2.height, 4);
}
