#include <gtest/gtest.h>
#include "draft/phys/shapes/polygon_shape.hpp"
#include <limits>

TEST(PolygonShape, DefaultConstructor)
{
    Draft::PolygonShape polygon;
    ASSERT_EQ(polygon.type, Draft::ShapeType::POLYGON);
    ASSERT_EQ(polygon.get_vertex_count(), 0);
}

TEST(PolygonShape, VertexManagement)
{
    Draft::PolygonShape polygon;

    // Add vertices
    polygon.add_vertex({0.f, 0.f});
    polygon.add_vertex({1.f, 0.f});
    polygon.add_vertex({1.f, 1.f});
    ASSERT_EQ(polygon.get_vertex_count(), 3);
    ASSERT_EQ(polygon.get_vertex(0).x, 0.f);
    ASSERT_EQ(polygon.get_vertex(2).y, 1.f);

    // Set vertex
    polygon.set_vertex(1, {0.5f, 0.5f});
    ASSERT_EQ(polygon.get_vertex(1).x, 0.5f);
    ASSERT_EQ(polygon.get_vertex(1).y, 0.5f);

    // Get all vertices
    const auto& vertices = polygon.get_vertices();
    ASSERT_EQ(vertices.size(), 3);

    // Delete vertex
    ASSERT_TRUE(polygon.del_vertex(1));
    ASSERT_EQ(polygon.get_vertex_count(), 2);
    ASSERT_EQ(polygon.get_vertex(0).x, 0.f);
    ASSERT_EQ(polygon.get_vertex(0).y, 0.f);
    ASSERT_EQ(polygon.get_vertex(1).x, 1.f);
    ASSERT_EQ(polygon.get_vertex(1).y, 1.f);

    ASSERT_FALSE(polygon.del_vertex(10)); // Invalid index
}

TEST(PolygonShape, SetAsBox)
{
    Draft::PolygonShape polygon;
    polygon.set_as_box(1.f, 2.f); // halfWidth, halfHeight

    ASSERT_EQ(polygon.get_vertex_count(), 4);
    // Vertices should be: (-1,-2), (1,-2), (1,2), (-1,2)
    std::vector<Draft::Vector2f> expected = { {-1.f, -2.f}, {1.f, -2.f}, {1.f, 2.f}, {-1.f, 2.f} };
    for (const auto& v_exp : expected) {
        bool found = false;
        for (const auto& v_poly : polygon.get_vertices()) {
            if (std::abs(v_exp.x - v_poly.x) < std::numeric_limits<float>::epsilon() &&
                std::abs(v_exp.y - v_poly.y) < std::numeric_limits<float>::epsilon()) {
                found = true;
                break;
            }
        }
        ASSERT_TRUE(found) << "Expected vertex {" << v_exp.x << "," << v_exp.y << "} not found.";
    }

    polygon.set_as_box(0.5f, 0.5f, 1.f, 1.f); // with center offset
    ASSERT_EQ(polygon.get_vertex_count(), 4);
    // Vertices should be: (0.5,0.5), (1.5,0.5), (1.5,1.5), (0.5,1.5)
    expected = { {0.5f, 0.5f}, {1.5f, 0.5f}, {1.5f, 1.5f}, {0.5f, 1.5f} };
    for (const auto& v_exp : expected) {
        bool found = false;
        for (const auto& v_poly : polygon.get_vertices()) {
            if (std::abs(v_exp.x - v_poly.x) < std::numeric_limits<float>::epsilon() &&
                std::abs(v_exp.y - v_poly.y) < std::numeric_limits<float>::epsilon()) {
                found = true;
                break;
            }
        }
        ASSERT_TRUE(found) << "Expected vertex {" << v_exp.x << "," << v_exp.y << "} not found.";
    }
}

TEST(PolygonShape, Clone)
{
    Draft::PolygonShape originalPolygon;
    originalPolygon.add_vertex({0.f, 0.f});
    originalPolygon.add_vertex({1.f, 0.f});
    originalPolygon.add_vertex({0.f, 1.f});
    originalPolygon.density = 0.5f;

    std::unique_ptr<Draft::Shape> clonedShape = originalPolygon.clone();
    Draft::PolygonShape* clonedPolygon = dynamic_cast<Draft::PolygonShape*>(clonedShape.get());

    ASSERT_NE(clonedPolygon, nullptr);
    ASSERT_EQ(clonedPolygon->type, originalPolygon.type);
    ASSERT_EQ(clonedPolygon->density, originalPolygon.density);
    ASSERT_EQ(clonedPolygon->get_vertex_count(), originalPolygon.get_vertex_count());
    for (size_t i = 0; i < originalPolygon.get_vertex_count(); ++i) {
        ASSERT_EQ(clonedPolygon->get_vertex(i).x, originalPolygon.get_vertex(i).x);
        ASSERT_EQ(clonedPolygon->get_vertex(i).y, originalPolygon.get_vertex(i).y);
    }
}

TEST(PolygonShape, Contains)
{
    Draft::PolygonShape polygon;
    polygon.set_as_box(1.f, 1.f); // Square from (-1,-1) to (1,1)

    ASSERT_TRUE(polygon.contains({0.f, 0.f}));
    ASSERT_TRUE(polygon.contains({0.5f, 0.5f}));
    ASSERT_TRUE(polygon.contains({-0.9f, 0.9f}));
    ASSERT_FALSE(polygon.contains({1.1f, 0.f}));
    ASSERT_FALSE(polygon.contains({-1.1f, -1.1f}));

    // Test with a triangle
    Draft::PolygonShape trianglePolygon;
    trianglePolygon.add_vertex({0.f, 0.f});
    trianglePolygon.add_vertex({2.f, 0.f});
    trianglePolygon.add_vertex({1.f, 2.f});

    ASSERT_TRUE(trianglePolygon.contains({1.f, 0.5f}));
    ASSERT_FALSE(trianglePolygon.contains({0.f, 1.f})); // Outside the triangle
    ASSERT_TRUE(trianglePolygon.contains({0.5f, 0.1f}));
    ASSERT_FALSE(trianglePolygon.contains({-0.1f, 0.f}));
}

// Regression test: the old engine had no bound check against box2d's hard [3,8] vertex limit
// before shape_to_b2() -> b2PolygonShape::Set() wrote into box2d's fixed b2Vec2[8] internal
// array
TEST(PolygonShape, ContainsThrowsOnTooManyVertices)
{
    Draft::PolygonShape polygon;
    for(int i = 0; i < 9; i++){
        // 9 vertices exceeds box2d's b2_maxPolygonVertices (8)
        polygon.add_vertex({static_cast<float>(i), static_cast<float>(i % 2)});
    }

    ASSERT_THROW(polygon.contains({0.f, 0.f}), std::runtime_error);
}

TEST(PolygonShape, ContainsThrowsOnTooFewVertices)
{
    Draft::PolygonShape polygon;
    polygon.add_vertex({0.f, 0.f});
    polygon.add_vertex({1.f, 0.f});

    ASSERT_THROW(polygon.contains({0.f, 0.f}), std::runtime_error);
}
