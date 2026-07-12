#include <gtest/gtest.h>
#include "draft/components/collider_component.hpp"
#include "draft/phys/shapes/circle_shape.hpp"

using namespace Draft;

TEST(ColliderComponent, DefaultConstructionHasNoShapes)
{
    ColliderComponent component;
    EXPECT_EQ(component.collider.get_shape_count(), 0);
    EXPECT_FALSE(component.collider.is_attached());
}

TEST(ColliderComponent, ShapeConstructorAddsTheGivenShape)
{
    CircleShape circle;
    circle.set_radius(2.f);

    ColliderComponent component(circle);

    ASSERT_EQ(component.collider.get_shape_count(), 1);
    EXPECT_EQ(component.collider.get_shapes()[0]->type, ShapeType::CIRCLE);
}

TEST(ColliderComponent, ColliderConstructorCopiesTheGivenCollider)
{
    Collider source;
    CircleShape circle;
    source.add_shape(circle);

    ColliderComponent component(source);

    EXPECT_EQ(component.collider.get_shape_count(), 1);
}

TEST(ColliderComponent, ArrowOperatorReachesTheUnderlyingCollider)
{
    ColliderComponent component;
    component->set_position({3.f, 4.f});

    EXPECT_FLOAT_EQ(component.collider.get_position().x, 3.f);
    EXPECT_FLOAT_EQ(component.collider.get_position().y, 4.f);
}
