#include <gtest/gtest.h>
#include "draft/rendering/batching/draw_command.hpp"

using namespace Draft;

TEST(SpriteDrawCommand, DefaultConstructedHasIdentityMatricesAndCleanFlag)
{
    SpriteDrawCommand cmd;
    EXPECT_EQ(cmd.projectionMatrix, Matrix4(1.f));
    EXPECT_EQ(cmd.transformMatrix, Matrix4(1.f));
    EXPECT_FALSE(cmd.matricesDirty);
}

TEST(SpriteDrawCommand, ComparatorOrdersByDescendingSpriteZIndex)
{
    SpriteDrawCommand low, high;
    low.sprite.zIndex = 1.f;
    high.sprite.zIndex = 5.f;

    SpriteDrawCommand comparator;
    EXPECT_TRUE(comparator(high, low));
    EXPECT_FALSE(comparator(low, high));
}

TEST(ShapeDrawCommand, DefaultConstructedHasLineTypeAndNoShader)
{
    ShapeDrawCommand cmd;
    EXPECT_TRUE(cmd.points.empty());
    EXPECT_EQ(cmd.type, ShapeRenderType::LINE);
    EXPECT_EQ(cmd.shader, nullptr);
    EXPECT_FALSE(cmd.matricesDirty);
    EXPECT_FALSE(cmd.shaderDirty);
    EXPECT_FALSE(cmd.layerDirty);
}

TEST(ShapeDrawCommand, PointsCanBeAccumulated)
{
    ShapeDrawCommand cmd;
    cmd.points.push_back({{0, 0}, {1, 1, 1, 1}});
    cmd.points.push_back({{1, 1}, {0, 0, 0, 1}});

    EXPECT_EQ(cmd.points.size(), 2u);
    EXPECT_FLOAT_EQ(cmd.points[1].position.x, 1.f);
}
