#include <gtest/gtest.h>
#include "draft/phys/raycast_props.hpp"
#include "draft/phys/shapes/circle_shape.hpp" // For testing ShapecastProps

TEST(RaycastProps, DefaultConstructor)
{
    Draft::RaycastProps props;
    ASSERT_EQ(props.origin.x, 0.f);
    ASSERT_EQ(props.origin.y, 0.f);
    ASSERT_EQ(props.translation.x, 0.f);
    ASSERT_EQ(props.translation.y, 0.f);
    ASSERT_EQ(props.maxFraction, 1.f);
}

TEST(RaycastProps, CustomValues)
{
    Draft::RaycastProps props;
    props.origin = {1.f, 2.f};
    props.translation = {3.f, 4.f};
    props.maxFraction = 0.5f;

    ASSERT_EQ(props.origin.x, 1.f);
    ASSERT_EQ(props.origin.y, 2.f);
    ASSERT_EQ(props.translation.x, 3.f);
    ASSERT_EQ(props.translation.y, 4.f);
    ASSERT_EQ(props.maxFraction, 0.5f);
}

TEST(ShapecastProps, Constructor)
{
    Draft::CircleShape dummyShape;
    Draft::ShapecastProps props {
        dummyShape,
        {1.f, 0.f},
        {0.f, 0.f},
        {5.f, 0.f},
        0.f,
        0.f
    };
    ASSERT_EQ(&props.shape, &dummyShape);
    ASSERT_EQ(props.direction.x, 1.f);
    ASSERT_EQ(props.direction.y, 0.f);
    ASSERT_EQ(props.originPos.x, 0.f);
    ASSERT_EQ(props.originPos.y, 0.f);
    ASSERT_EQ(props.translationPos.x, 5.f);
    ASSERT_EQ(props.translationPos.y, 0.f);
    ASSERT_EQ(props.originRot, 0.f);
    ASSERT_EQ(props.translationRot, 0.f);
}

TEST(RaycastResult, DefaultConstructor)
{
    Draft::RaycastResult result;
    ASSERT_EQ(result.normal.x, 0.f);
    ASSERT_EQ(result.normal.y, 0.f);
    ASSERT_EQ(result.fraction, 0.f);
}

TEST(RaycastResult, CustomValues)
{
    Draft::RaycastResult result;
    result.normal = {0.f, 1.f};
    result.fraction = 0.75f;

    ASSERT_EQ(result.normal.x, 0.f);
    ASSERT_EQ(result.normal.y, 1.f);
    ASSERT_EQ(result.fraction, 0.75f);
}
