#include <gtest/gtest.h>
#include "draft/components/collider_component.hpp"
#include "draft/physics/shapes/circle_shape.hpp"
#include "draft/util/serialization/serializer.hpp"

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

// Collider has its own explicit serialize/deserialize, and ColliderComponent is
// DRAFT_REFLECTABLE, so the reflect tier picks Collider's methods up with no extra code.
static_assert(Serializer::JsonSerializable<Collider>);
static_assert(Serializer::BinarySerializable<Collider>);
static_assert(Reflectable<ColliderComponent>);
static_assert(!Serializer::JsonSerializable<ColliderComponent>);

TEST(ColliderComponent, JsonRoundTripReachesTheUnderlyingCollider)
{
    CircleShape circle;
    circle.set_radius(2.f);
    ColliderComponent original(circle);

    JSON json;
    Serializer::serialize(original, json);

    ColliderComponent restored;
    Serializer::deserialize(restored, json);

    ASSERT_EQ(restored.collider.get_shape_count(), 1);
    EXPECT_EQ(restored.collider.get_shapes()[0]->type, ShapeType::CIRCLE);
}
