#include <gtest/gtest.h>
#include "draft/math/color.hpp"
#include <stdexcept>

TEST(Color, FromHex)
{
    Draft::Color c1("#ff0000");
    ASSERT_EQ(c1.r, 1.f);
    ASSERT_EQ(c1.g, 0.f);
    ASSERT_EQ(c1.b, 0.f);
    ASSERT_EQ(c1.a, 1.f);

    Draft::Color c2("ff0000");
    ASSERT_EQ(c2.r, 1.f);
    ASSERT_EQ(c2.g, 0.f);
    ASSERT_EQ(c2.b, 0.f);
    ASSERT_EQ(c2.a, 1.f);

    Draft::Color c3("#ff0000ff");
    ASSERT_EQ(c3.r, 1.f);
    ASSERT_EQ(c3.g, 0.f);
    ASSERT_EQ(c3.b, 0.f);
    ASSERT_EQ(c3.a, 1.f);

    Draft::Color c4("ff0000ff");
    ASSERT_EQ(c4.r, 1.f);
    ASSERT_EQ(c4.g, 0.f);
    ASSERT_EQ(c4.b, 0.f);
    ASSERT_EQ(c4.a, 1.f);
}

TEST(Color, FromShorthandHex)
{
    Draft::Color c1("#f00");
    ASSERT_EQ(c1.r, 1.f);
    ASSERT_EQ(c1.g, 0.f);
    ASSERT_EQ(c1.b, 0.f);
    ASSERT_EQ(c1.a, 1.f);

    Draft::Color c2("#f00f");
    ASSERT_EQ(c2.r, 1.f);
    ASSERT_EQ(c2.g, 0.f);
    ASSERT_EQ(c2.b, 0.f);
    ASSERT_EQ(c2.a, 1.f);
}

TEST(Color, ToHex)
{
    Draft::Color c1(1.f, 0.f, 0.f, 1.f);
    ASSERT_EQ(c1.to_hex(), "#ff0000");

    Draft::Color c2(1.f, 0.f, 0.f, 1.f);
    ASSERT_EQ(c2.to_hex(true), "#ff0000ff");
}

TEST(Color, InvalidHexLengthThrows)
{
    ASSERT_THROW(Draft::Color("#ff"), std::invalid_argument);
    ASSERT_THROW(Draft::Color("#ff00000"), std::invalid_argument);
    ASSERT_THROW(Draft::Color(""), std::invalid_argument);
}

TEST(Color, NamedConstants)
{
    ASSERT_EQ(Draft::Color::WHITE.to_hex(), "#f0f0f0");
    ASSERT_EQ(Draft::Color::BLACK.to_hex(), "#111111");
}
