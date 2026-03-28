#include <gtest/gtest.h>
#include "draft/util/time.hpp"

TEST(Time, Constructors)
{
    Draft::Time t1 = Draft::Time::seconds(1.5f);
    ASSERT_EQ(t1.as_seconds(), 1.5f);
    ASSERT_EQ(t1.as_milliseconds(), 1500);
    ASSERT_EQ(t1.as_microseconds(), 1500000);

    Draft::Time t2 = Draft::Time::milliseconds(1500);
    ASSERT_EQ(t2.as_seconds(), 1.5f);
    ASSERT_EQ(t2.as_milliseconds(), 1500);
    ASSERT_EQ(t2.as_microseconds(), 1500000);

    Draft::Time t3 = Draft::Time::microseconds(1500000);
    ASSERT_EQ(t3.as_seconds(), 1.5f);
    ASSERT_EQ(t3.as_milliseconds(), 1500);
    ASSERT_EQ(t3.as_microseconds(), 1500000);
}

TEST(Time, Operators)
{
    Draft::Time t1 = Draft::Time::seconds(1.f);
    Draft::Time t2 = Draft::Time::seconds(2.f);

    ASSERT_TRUE(t1 != t2);
    ASSERT_TRUE(t1 < t2);
    ASSERT_TRUE(t2 > t1);
    ASSERT_TRUE(t1 <= t2);
    ASSERT_TRUE(t2 >= t1);

    Draft::Time t3 = t1 + t2;
    ASSERT_EQ(t3.as_seconds(), 3.f);

    Draft::Time t4 = t2 - t1;
    ASSERT_EQ(t4.as_seconds(), 1.f);

    Draft::Time t5 = t1 * 2.f;
    ASSERT_EQ(t5.as_seconds(), 2.f);

    Draft::Time t6 = t2 / 2.f;
    ASSERT_EQ(t6.as_seconds(), 1.f);
}

TEST(Time, Literals)
{
    using namespace Draft;
    Draft::Time t1 = 1.5_s;
    ASSERT_EQ(t1.as_seconds(), 1.5f);
    
    Draft::Time t2 = 1500_ms;
    ASSERT_EQ(t2.as_milliseconds(), 1500);

    Draft::Time t3 = 1500000_mis;
    ASSERT_EQ(t3.as_microseconds(), 1500000);
}
