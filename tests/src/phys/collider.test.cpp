#include <gtest/gtest.h>
#include "draft/phys/collider.hpp"
#include "draft/phys/shapes/circle_shape.hpp"
#include "draft/phys/shapes/polygon_shape.hpp"
#include "draft/phys/world.hpp"
#include "draft/phys/rigid_body.hpp"
#include "draft/phys/body_def.hpp"
#include "draft/util/json.hpp"

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

    // Capture the pointers to the *cloned* shapes owned by the collider
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

TEST(Collider, JsonConstructor)
{
    // Example JSON string for a collider
    std::string json_str = R"(
        [
            {
                "indices": [0, 1, 2, 3],
                "vertices": [
                    -0.5, -0.5,
                    0.5, -0.5,
                    0.5, 0.5,
                    -0.5, 0.5
                ],
                "restitution": 0.3,
                "density": 1.0,
                "friction": 0.5,
                "sensor": false,
                "convex": true
            }
        ]
    )";
    Draft::JSON json_obj = Draft::JSON::parse(json_str);

    Draft::Collider collider(json_obj);

    // Assert that the collider now has one shape and verify its properties
    ASSERT_EQ(collider.get_shape_count(), 1);
    const Draft::Shape* shape = collider.get_shapes()[0].get();
    ASSERT_EQ(shape->type, Draft::ShapeType::POLYGON);

    // Cast to PolygonShape to access its specific properties
    const Draft::PolygonShape* polygonShape = dynamic_cast<const Draft::PolygonShape*>(shape);
    ASSERT_NE(polygonShape, nullptr);

    // Verify properties loaded from JSON
    ASSERT_EQ(polygonShape->restitution, 0.3f);
    ASSERT_EQ(polygonShape->density, 1.0f);
    ASSERT_EQ(polygonShape->friction, 0.5f);
    ASSERT_FALSE(polygonShape->isSensor); // sensor should be false from JSON
    ASSERT_TRUE(polygonShape->isConvex); // convex should be true from JSON

    // Verify vertices
    ASSERT_EQ(polygonShape->get_vertex_count(), 4);
    ASSERT_EQ(polygonShape->get_vertex(0).x, -0.5f);
    ASSERT_EQ(polygonShape->get_vertex(0).y, -0.5f);
    ASSERT_EQ(polygonShape->get_vertex(1).x, 0.5f);
    ASSERT_EQ(polygonShape->get_vertex(1).y, -0.5f);
    ASSERT_EQ(polygonShape->get_vertex(2).x, 0.5f);
    ASSERT_EQ(polygonShape->get_vertex(2).y, 0.5f);
    ASSERT_EQ(polygonShape->get_vertex(3).x, -0.5f);
    ASSERT_EQ(polygonShape->get_vertex(3).y, 0.5f);
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
