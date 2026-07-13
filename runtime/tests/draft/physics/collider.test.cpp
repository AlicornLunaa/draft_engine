#include <gtest/gtest.h>
#include "draft/physics/collider.hpp"
#include "draft/physics/shapes/circle_shape.hpp"
#include "draft/physics/shapes/polygon_shape.hpp"
#include "draft/physics/shapes/edge_shape.hpp"
#include "draft/physics/shapes/chain_shape.hpp"
#include "draft/physics/world.hpp"
#include "draft/physics/rigid_body.hpp"
#include "draft/physics/body_def.hpp"

TEST(Collider, DefaultConstructor)
{
    Draft::Collider collider;
    ASSERT_EQ(collider.get_shape_count(), 0);
    ASSERT_EQ(collider.get_position().x, 0.f);
    ASSERT_EQ(collider.get_position().y, 0.f);
    ASSERT_EQ(collider.get_origin().x, 0.f);
    ASSERT_EQ(collider.get_origin().y, 0.f);
    ASSERT_EQ(collider.get_scale().x, 1.f);
    ASSERT_EQ(collider.get_scale().y, 1.f);
    ASSERT_EQ(collider.get_rotation(), 0.f);
    ASSERT_FALSE(collider.is_attached());
    ASSERT_EQ(collider.get_body(), nullptr);
}

TEST(Collider, ShapeManagement)
{
    Draft::Collider collider;
    Draft::CircleShape circleTemplate;
    Draft::PolygonShape polygonTemplate;

    // Capture the pointers to the cloned shapes owned by the collider
    Draft::Shape* addedCircle = collider.add_shape(circleTemplate);
    ASSERT_EQ(collider.get_shape_count(), 1);
    ASSERT_EQ(addedCircle->type, Draft::ShapeType::CIRCLE);

    Draft::Shape* addedPolygon = collider.add_shape(polygonTemplate);
    ASSERT_EQ(collider.get_shape_count(), 2);
    ASSERT_EQ(addedPolygon->type, Draft::ShapeType::POLYGON);

    ASSERT_EQ(collider.get_shapes()[0]->type, Draft::ShapeType::CIRCLE);
    ASSERT_EQ(collider.get_shapes()[1]->type, Draft::ShapeType::POLYGON);

    collider.del_shape(addedCircle);
    ASSERT_EQ(collider.get_shape_count(), 1);
    ASSERT_EQ(collider.get_shapes()[0]->type, Draft::ShapeType::POLYGON);

    collider.clear();
    ASSERT_EQ(collider.get_shape_count(), 0);
}

TEST(Collider, Transformations)
{
    Draft::Collider collider;

    collider.set_position({10.f, 20.f});
    ASSERT_EQ(collider.get_position().x, 10.f);
    ASSERT_EQ(collider.get_position().y, 20.f);

    collider.set_origin({5.f, 5.f});
    ASSERT_EQ(collider.get_origin().x, 5.f);
    ASSERT_EQ(collider.get_origin().y, 5.f);

    collider.set_scale({2.f, 3.f});
    ASSERT_EQ(collider.get_scale().x, 2.f);
    ASSERT_EQ(collider.get_scale().y, 3.f);

    collider.set_scale(4.f);
    ASSERT_EQ(collider.get_scale().x, 4.f);
    ASSERT_EQ(collider.get_scale().y, 4.f);

    collider.set_rotation(45.f);
    ASSERT_EQ(collider.get_rotation(), 45.f);
}

TEST(Collider, AttachDetach)
{
    Draft::World world({0.f, 0.f});
    Draft::BodyDef bodyDef;
    Draft::RigidBody* body = world.create_rigid_body(bodyDef);

    Draft::Collider collider;
    ASSERT_FALSE(collider.is_attached());
    ASSERT_EQ(collider.get_body(), nullptr);

    collider.attach(body);
    ASSERT_TRUE(collider.is_attached());
    ASSERT_EQ(collider.get_body(), body);

    collider.detach();
    ASSERT_FALSE(collider.is_attached());
    ASSERT_EQ(collider.get_body(), nullptr);

    world.destroy_body(body);
}

TEST(Collider, TestPoint)
{
    Draft::Collider collider;
    Draft::CircleShape circleTemplate;
    circleTemplate.set_radius(1.0f);
    collider.add_shape(circleTemplate); // A circle at {0,0} with radius 1

    // Test with no transformation (default)
    ASSERT_TRUE(collider.test_point({0.f, 0.f}));
    ASSERT_TRUE(collider.test_point({0.5f, 0.f}));
    ASSERT_FALSE(collider.test_point({1.1f, 0.f}));

    // Test with position offset
    collider.set_position({10.f, 0.f});
    ASSERT_TRUE(collider.test_point({10.f, 0.f}));
    ASSERT_TRUE(collider.test_point({10.5f, 0.f}));
    ASSERT_FALSE(collider.test_point({11.1f, 0.f}));
    ASSERT_FALSE(collider.test_point({0.f, 0.f})); // Old center

    // Test with scale
    collider.set_position({0.f, 0.f}); // Reset position
    collider.set_scale({2.f, 2.f}); // Radius becomes 2
    ASSERT_TRUE(collider.test_point({0.f, 0.f}));
    ASSERT_TRUE(collider.test_point({1.5f, 0.f}));
    ASSERT_FALSE(collider.test_point({2.1f, 0.f}));

    // Test with origin offset and rotation
    collider.set_scale({1.f, 1.f}); // Reset scale
    collider.set_origin({0.5f, 0.5f}); // Center of circle at {0.5, 0.5} (relative to collider's position)
    collider.set_rotation(90.f); // Rotate by 90 degrees around origin
    collider.set_position({1.f, 1.f}); // Collider at {1,1}

    ASSERT_TRUE(collider.test_point({1.5f, 1.5f}));
    ASSERT_TRUE(collider.test_point({1.f, 1.5f})); // Within radius 1 from {1.5, 1.5}
    ASSERT_FALSE(collider.test_point({0.f, 0.f}));
}

// Regression: the old engine's Collider had no user-declared destructor, so destroying an
// attached Collider while its RigidBody stayed alive never destroyed the corresponding box2d
// fixtures
TEST(Collider, DestroyingAttachedColliderDestroysItsFixtures)
{
    Draft::World world({0.f, 0.f});
    Draft::BodyDef bodyDef;
    Draft::RigidBody* body = world.create_rigid_body(bodyDef);

    {
        Draft::Collider collider;
        Draft::CircleShape circleTemplate;
        collider.add_shape(circleTemplate);
        collider.attach(body);
        ASSERT_EQ(body->get_fixture_list().size(), 1);
    } // collider destructs here while still attached to `body`

    ASSERT_EQ(body->get_fixture_list().size(), 0);

    world.destroy_body(body);
}

// Regression: nothing invalidated a Collider still pointing at a RigidBody that got destroyed
// collider.set_position() -> update_collider() -> attach() would dereference the dead body, a
// real use-after-free. RigidBody's destructor now invalidates every still-attached Collider first.
TEST(Collider, ColliderIsInvalidatedWhenItsBodyIsDestroyedFirst)
{
    Draft::World world({0.f, 0.f});
    Draft::BodyDef bodyDef;
    Draft::RigidBody* body = world.create_rigid_body(bodyDef);

    Draft::Collider collider;
    Draft::CircleShape circleTemplate;
    collider.add_shape(circleTemplate);
    collider.attach(body);
    ASSERT_TRUE(collider.is_attached());

    world.destroy_body(body);

    ASSERT_FALSE(collider.is_attached());
    ASSERT_EQ(collider.get_body(), nullptr);

    // Would dereference the dead RigidBody via update_collider() -> attach() without the fix.
    collider.set_position({5.f, 5.f});
    ASSERT_EQ(collider.get_position().x, 5.f);
    ASSERT_EQ(collider.get_position().y, 5.f);
}

namespace {
    Draft::Collider binary_round_trip(const Draft::Collider& original){
        Draft::Binary::ByteArray buffer;
        Draft::Collider::serialize(original, buffer);

        Draft::Binary::ByteView view(buffer);
        Draft::Collider restored;
        Draft::Collider::deserialize(restored, view);
        return restored;
    }

    Draft::Collider json_round_trip(const Draft::Collider& original){
        Draft::JSON json;
        Draft::Collider::serialize(original, json);

        Draft::Collider restored;
        Draft::Collider::deserialize(restored, json);
        return restored;
    }

    void assert_shapes_match(const Draft::Shape* a, const Draft::Shape* b){
        ASSERT_EQ(a->type, b->type);
        ASSERT_FLOAT_EQ(a->friction, b->friction);
        ASSERT_FLOAT_EQ(a->restitution, b->restitution);
        ASSERT_FLOAT_EQ(a->density, b->density);
        ASSERT_EQ(a->isSensor, b->isSensor);
        ASSERT_EQ(a->isConvex, b->isConvex);
    }
}

TEST(ColliderSerialization, CircleRoundTrip)
{
    Draft::Collider original;
    Draft::CircleShape shape;
    shape.set_position({1.f, 2.f});
    shape.set_radius(3.f);
    shape.isSensor = true;
    original.add_shape(shape);

    for(auto& restored : {binary_round_trip(original), json_round_trip(original)}){
        ASSERT_EQ(restored.get_shape_count(), 1);

        auto* circle = static_cast<Draft::CircleShape*>(restored.get_shapes()[0].get());
        assert_shapes_match(original.get_shapes()[0].get(), circle);
        ASSERT_FLOAT_EQ(circle->get_position().x, 1.f);
        ASSERT_FLOAT_EQ(circle->get_position().y, 2.f);
        ASSERT_FLOAT_EQ(circle->get_radius(), 3.f);
    }
}

TEST(ColliderSerialization, PolygonRoundTrip)
{
    Draft::Collider original;
    Draft::PolygonShape shape;
    shape.add_vertex({0.f, 0.f});
    shape.add_vertex({1.f, 0.f});
    shape.add_vertex({1.f, 1.f});
    original.add_shape(shape);

    for(auto& restored : {binary_round_trip(original), json_round_trip(original)}){
        ASSERT_EQ(restored.get_shape_count(), 1);

        auto* polygon = static_cast<Draft::PolygonShape*>(restored.get_shapes()[0].get());
        assert_shapes_match(original.get_shapes()[0].get(), polygon);
        ASSERT_EQ(polygon->get_vertex_count(), 3u);
        ASSERT_FLOAT_EQ(polygon->get_vertex(1).x, 1.f);
        ASSERT_FLOAT_EQ(polygon->get_vertex(2).y, 1.f);
    }
}

TEST(ColliderSerialization, EdgeRoundTrip)
{
    Draft::Collider original;
    Draft::EdgeShape shape({-1.f, 0.f}, {1.f, 0.f});
    original.add_shape(shape);

    for(auto& restored : {binary_round_trip(original), json_round_trip(original)}){
        ASSERT_EQ(restored.get_shape_count(), 1);

        auto* edge = static_cast<Draft::EdgeShape*>(restored.get_shapes()[0].get());
        assert_shapes_match(original.get_shapes()[0].get(), edge);
        ASSERT_FLOAT_EQ(edge->get_start().x, -1.f);
        ASSERT_FLOAT_EQ(edge->get_end().x, 1.f);
    }
}

TEST(ColliderSerialization, ChainRoundTrip)
{
    Draft::Collider original;
    Draft::ChainShape shape(Draft::ChainShape::LOOP);
    shape.add({0.f, 0.f});
    shape.add({1.f, 0.f});
    shape.add({1.f, 1.f});
    shape.set_previous({-1.f, -1.f});
    shape.set_next({2.f, 2.f});
    original.add_shape(shape);

    for(auto& restored : {binary_round_trip(original), json_round_trip(original)}){
        ASSERT_EQ(restored.get_shape_count(), 1);

        auto* chain = static_cast<Draft::ChainShape*>(restored.get_shapes()[0].get());
        assert_shapes_match(original.get_shapes()[0].get(), chain);
        ASSERT_EQ(chain->get_chain_type(), Draft::ChainShape::LOOP);
        ASSERT_EQ(chain->get_points().size(), 3u);
        ASSERT_FLOAT_EQ(chain->get_previous().x, -1.f);
        ASSERT_FLOAT_EQ(chain->get_next().y, 2.f);
    }
}

TEST(ColliderSerialization, MultiShapeAndTransformRoundTrip)
{
    Draft::Collider original;
    Draft::CircleShape circle;
    circle.set_radius(2.f);
    original.add_shape(circle);
    original.add_shape(Draft::PolygonShape{});

    original.set_position({3.f, 4.f});
    original.set_origin({0.5f, 0.5f});
    original.set_scale({2.f, 2.f});
    original.set_rotation(1.2f);

    for(auto& restored : {binary_round_trip(original), json_round_trip(original)}){
        ASSERT_EQ(restored.get_shape_count(), 2);
        ASSERT_EQ(restored.get_shapes()[0]->type, Draft::ShapeType::CIRCLE);
        ASSERT_EQ(restored.get_shapes()[1]->type, Draft::ShapeType::POLYGON);

        ASSERT_FLOAT_EQ(restored.get_position().x, 3.f);
        ASSERT_FLOAT_EQ(restored.get_position().y, 4.f);
        ASSERT_FLOAT_EQ(restored.get_origin().x, 0.5f);
        ASSERT_FLOAT_EQ(restored.get_scale().x, 2.f);
        ASSERT_FLOAT_EQ(restored.get_rotation(), 1.2f);
    }
}
