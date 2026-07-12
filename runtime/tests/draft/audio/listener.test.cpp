#include <gtest/gtest.h>
#include "draft/audio/listener.hpp"

using namespace Draft;

TEST(Listener, DefaultsMatchAnUnrotatedForwardListener)
{
    Listener listener;

    EXPECT_FLOAT_EQ(listener.get_position().x, 0.f);
    EXPECT_FLOAT_EQ(listener.get_direction().z, 1.f);
    EXPECT_FLOAT_EQ(listener.get_up().y, 1.f);
    EXPECT_FLOAT_EQ(listener.get_volume(), 1.f);
}

TEST(Listener, ConstructorTakesAnInitialPositionAndVolume)
{
    Listener listener({1.f, 2.f, 3.f}, 0.5f);

    EXPECT_FLOAT_EQ(listener.get_position().x, 1.f);
    EXPECT_FLOAT_EQ(listener.get_position().y, 2.f);
    EXPECT_FLOAT_EQ(listener.get_position().z, 3.f);
    EXPECT_FLOAT_EQ(listener.get_volume(), 0.5f);
}

TEST(Listener, SettersRoundTrip)
{
    Listener listener;
    listener.set_position({4.f, 5.f, 6.f});
    listener.set_direction({0.f, 0.f, -1.f});
    listener.set_up({0.f, 0.f, 1.f});
    listener.set_volume(75.f);

    EXPECT_FLOAT_EQ(listener.get_position().x, 4.f);
    EXPECT_FLOAT_EQ(listener.get_direction().z, -1.f);
    EXPECT_FLOAT_EQ(listener.get_up().z, 1.f);
    EXPECT_FLOAT_EQ(listener.get_volume(), 75.f);
}

TEST(Listener, ApplyDoesNotThrow)
{
    Listener listener({1.f, 0.f, 0.f}, 50.f);
    ASSERT_NO_THROW(listener.apply());
}
