#include <gtest/gtest.h>
#include "draft/util/time.hpp"

using namespace Draft;

TEST(Time, DefaultIsZero)
{
    Time t;
    EXPECT_EQ(t.as_microseconds(), 0);
    EXPECT_EQ(t.as_milliseconds(), 0);
    EXPECT_FLOAT_EQ(t.as_seconds(), 0.f);
}

TEST(Time, CopyConstructionPreservesValue)
{
    Time original = Time::milliseconds(750);
    Time copy(original);
    EXPECT_EQ(copy.as_microseconds(), original.as_microseconds());
}

TEST(Time, SecondsFactory)
{
    Time t = Time::seconds(1.5f);
    EXPECT_EQ(t.as_microseconds(), 1500000);
    EXPECT_EQ(t.as_milliseconds(), 1500);
    EXPECT_FLOAT_EQ(t.as_seconds(), 1.5f);
}

TEST(Time, MillisecondsFactory)
{
    Time t = Time::milliseconds(500);
    EXPECT_EQ(t.as_microseconds(), 500000);
    EXPECT_EQ(t.as_milliseconds(), 500);
    EXPECT_FLOAT_EQ(t.as_seconds(), 0.5f);
}

TEST(Time, MicrosecondsFactory)
{
    Time t = Time::microseconds(250);
    EXPECT_EQ(t.as_microseconds(), 250);
    EXPECT_EQ(t.as_milliseconds(), 0);
}

TEST(Time, NegativeFactoriesPreserveSign)
{
    EXPECT_EQ(Time::seconds(-2.f).as_microseconds(), -2000000);
    EXPECT_EQ(Time::milliseconds(-500).as_microseconds(), -500000);
    EXPECT_EQ(Time::microseconds(-100).as_microseconds(), -100);
}

TEST(Time, AsMillisecondsTruncatesTowardZero)
{
    // 1999us is 1.999ms - truncated (not rounded) down to 1ms, and symmetrically for negatives.
    EXPECT_EQ(Time::microseconds(1999).as_milliseconds(), 1);
    EXPECT_EQ(Time::microseconds(-1999).as_milliseconds(), -1);
}

TEST(Time, SubMicrosecondFractionsAreTruncated)
{
    // Time only stores whole microseconds, anything finer than that floors toward zero rather
    // than rounding, same as the truncation above.
    EXPECT_EQ(Time::seconds(0.0000001f).as_microseconds(), 0);
}

TEST(Time, EqualityAndOrdering)
{
    Time a = Time::milliseconds(100);
    Time b = Time::milliseconds(100);
    Time c = Time::milliseconds(200);

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_TRUE(a != c);
    EXPECT_FALSE(a != b);

    EXPECT_TRUE(a < c);
    EXPECT_TRUE(c > a);
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(a >= b);
    EXPECT_FALSE(c < a);
    EXPECT_FALSE(a > c);
}

TEST(Time, AssignmentOperator)
{
    Time a = Time::seconds(1.f);
    Time b = Time::seconds(5.f);
    a = b;
    EXPECT_EQ(a.as_microseconds(), b.as_microseconds());
}

TEST(Time, UnaryNegation)
{
    Time t = Time::seconds(3.f);
    Time neg = -t;
    EXPECT_EQ(neg.as_microseconds(), -3000000);
    EXPECT_EQ((-neg).as_microseconds(), t.as_microseconds());
}

TEST(Time, AdditionAndSubtraction)
{
    Time a = Time::milliseconds(300);
    Time b = Time::milliseconds(200);

    EXPECT_EQ((a + b).as_milliseconds(), 500);
    EXPECT_EQ((a - b).as_milliseconds(), 100);
    EXPECT_EQ((b - a).as_milliseconds(), -100);
}

TEST(Time, CompoundAdditionAndSubtraction)
{
    Time t = Time::milliseconds(100);

    t += Time::milliseconds(50);
    EXPECT_EQ(t.as_milliseconds(), 150);

    t -= Time::milliseconds(70);
    EXPECT_EQ(t.as_milliseconds(), 80);
}

TEST(Time, ScalarMultiplication)
{
    Time t = Time::milliseconds(100);

    EXPECT_EQ((t * 2.5f).as_microseconds(), 250000);
    EXPECT_EQ((t * static_cast<int64_t>(3)).as_microseconds(), 300000);
}

TEST(Time, ScalarDivision)
{
    Time t = Time::milliseconds(100);

    EXPECT_EQ((t / 2.f).as_microseconds(), 50000);
    EXPECT_EQ((t / static_cast<int64_t>(4)).as_microseconds(), 25000);
}

TEST(Time, CompoundScalarMultiplicationAndDivision)
{
    Time t = Time::milliseconds(100);

    t *= 2.f;
    EXPECT_EQ(t.as_milliseconds(), 200);

    t /= static_cast<int64_t>(4);
    EXPECT_EQ(t.as_milliseconds(), 50);

    t *= static_cast<int64_t>(3);
    EXPECT_EQ(t.as_milliseconds(), 150);

    t /= 3.f;
    EXPECT_EQ(t.as_milliseconds(), 50);
}

TEST(Time, RatioToAnotherTimeIsExactForCleanDivisions)
{
    Time a = Time::seconds(3.f);
    Time b = Time::seconds(1.5f);
    EXPECT_FLOAT_EQ(a / b, 2.f);
}

TEST(Time, RatioToAnotherTimeIsNotIntegerTruncated)
{
    // If this delegated to duration/duration (both int64_t rep), 1/3 would truncate to 0.
    Time a = Time::microseconds(1);
    Time b = Time::microseconds(3);
    EXPECT_NEAR(a / b, 1.f / 3.f, 1e-6f);
}

TEST(Time, Modulo)
{
    Time a = Time::milliseconds(2500);
    Time b = Time::seconds(1.f);
    EXPECT_EQ((a % b).as_milliseconds(), 500);
}

TEST(Time, CompoundModulo)
{
    Time a = Time::milliseconds(2500);
    a %= Time::seconds(1.f);
    EXPECT_EQ(a.as_milliseconds(), 500);
}

TEST(Time, Literals)
{
    Time bySeconds = 1.5_s;
    Time byMillis = 500_ms;
    Time byMicros = 250_mis;

    EXPECT_EQ(bySeconds.as_microseconds(), Time::seconds(1.5f).as_microseconds());
    EXPECT_EQ(byMillis.as_microseconds(), Time::milliseconds(500).as_microseconds());
    EXPECT_EQ(byMicros.as_microseconds(), Time::microseconds(250).as_microseconds());
}
