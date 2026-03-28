#include <gtest/gtest.h>
#include "draft/util/color.hpp"

TEST(Color, FromHex)
{
    Color c1("#ff0000");
    ASSERT_EQ(c1.r, 1.f);
    ASSERT_EQ(c1.g, 0.f);
    ASSERT_EQ(c1.b, 0.f);
    ASSERT_EQ(c1.a, 1.f);

    Color c2("ff0000");
    ASSERT_EQ(c2.r, 1.f);
    ASSERT_EQ(c2.g, 0.f);
    ASSERT_EQ(c2.b, 0.f);
    ASSERT_EQ(c2.a, 1.f);

    Color c3("#ff0000ff");
    ASSERT_EQ(c3.r, 1.f);
    ASSERT_EQ(c3.g, 0.f);
    ASSERT_EQ(c3.b, 0.f);
    ASSERT_EQ(c3.a, 1.f);

    Color c4("ff0000ff");
    ASSERT_EQ(c4.r, 1.f);
    ASSERT_EQ(c4.g, 0.f);
    ASSERT_EQ(c4.b, 0.f);
    ASSERT_EQ(c4.a, 1.f);
}

TEST(Color, ToHex)
{
    Color c1(1.f, 0.f, 0.f, 1.f);
    ASSERT_EQ(c1.to_hex(), "#ff0000");

    Color c2(1.f, 0.f, 0.f, 1.f);
    ASSERT_EQ(c2.to_hex(true), "#ff0000ff");
}
