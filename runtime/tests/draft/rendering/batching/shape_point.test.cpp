#include <gtest/gtest.h>
#include "draft/rendering/batching/shape_point.hpp"

using namespace Draft;

TEST(ShapePoint, DefaultConstructedHasOriginPositionAndWhiteOpaqueColor)
{
    ShapePoint point;
    EXPECT_FLOAT_EQ(point.position.x, 0.f);
    EXPECT_FLOAT_EQ(point.position.y, 0.f);
    EXPECT_FLOAT_EQ(point.color.r, 1.f);
    EXPECT_FLOAT_EQ(point.color.a, 1.f);
}

TEST(ShapePoint, FieldsAreAssignable)
{
    ShapePoint point;
    point.position = {3.f, 4.f};
    point.color = {0.f, 1.f, 0.f, 0.5f};

    EXPECT_FLOAT_EQ(point.position.x, 3.f);
    EXPECT_FLOAT_EQ(point.position.y, 4.f);
    EXPECT_FLOAT_EQ(point.color.g, 1.f);
    EXPECT_FLOAT_EQ(point.color.a, 0.5f);
}

TEST(ShapeRenderType, HasFillAndLineValues)
{
    ShapeRenderType fill = ShapeRenderType::FILL;
    ShapeRenderType line = ShapeRenderType::LINE;
    EXPECT_NE(fill, line);
}
