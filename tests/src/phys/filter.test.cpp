#include <gtest/gtest.h>
#include "draft/phys/filter.hpp"

TEST(Filter, DefaultConstructor)
{
    Draft::PhysMask mask;
    ASSERT_EQ(mask.categoryBits, 0x0001);
    ASSERT_EQ(mask.maskBits, 0xFFFF);
    ASSERT_EQ(mask.groupIndex, 0);
}

TEST(Filter, CustomValues)
{
    Draft::PhysMask mask;
    mask.categoryBits = 0x0002;
    mask.maskBits = 0xFF00;
    mask.groupIndex = 5;

    ASSERT_EQ(mask.categoryBits, 0x0002);
    ASSERT_EQ(mask.maskBits, 0xFF00);
    ASSERT_EQ(mask.groupIndex, 5);
}
