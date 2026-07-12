#include <gtest/gtest.h>
#include "draft/rendering/batching/sprite_props.hpp"

using namespace Draft;

TEST(SpriteProps, DefaultConstructedHasUnitScaleAndZeroIndex)
{
    SpriteProps props;
    EXPECT_FLOAT_EQ(props.position.x, 0.f);
    EXPECT_FLOAT_EQ(props.size.x, 1.f);
    EXPECT_FLOAT_EQ(props.size.y, 1.f);
    EXPECT_FLOAT_EQ(props.zIndex, 0.f);
    EXPECT_FLOAT_EQ(props.rotation, 0.f);
}

TEST(SpriteProps, ComparatorOrdersByDescendingZIndex)
{
    SpriteProps low, high;
    low.zIndex = 1.f;
    high.zIndex = 5.f;

    SpriteProps comparator;
    EXPECT_TRUE(comparator(high, low)); // high.zIndex > low.zIndex -> true
    EXPECT_FALSE(comparator(low, high));
}
