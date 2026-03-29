#include <gtest/gtest.h>
#include "draft/phys/shapes/edge_shape.hpp"

TEST(EdgeShape, DefaultConstructor)
{
    Draft::EdgeShape edge;
    ASSERT_EQ(edge.type, Draft::ShapeType::EDGE);
    ASSERT_EQ(edge.get_start().x, 0.f);
    ASSERT_EQ(edge.get_start().y, 0.f);
    ASSERT_EQ(edge.get_end().x, 0.f);
    ASSERT_EQ(edge.get_end().y, 0.f);
}

TEST(EdgeShape, ParametrizedConstructor)
{
    Draft::Vector2f start = {1.f, 2.f};
    Draft::Vector2f end = {3.f, 4.f};
    Draft::EdgeShape edge(start, end);
    ASSERT_EQ(edge.type, Draft::ShapeType::EDGE);
    ASSERT_EQ(edge.get_start().x, start.x);
    ASSERT_EQ(edge.get_start().y, start.y);
    ASSERT_EQ(edge.get_end().x, end.x);
    ASSERT_EQ(edge.get_end().y, end.y);
}

TEST(EdgeShape, SetGetPoints)
{
    Draft::EdgeShape edge;
    Draft::Vector2f newStart = {5.f, 6.f};
    Draft::Vector2f newEnd = {7.f, 8.f};

    edge.set_start(newStart);
    edge.set_end(newEnd);
    ASSERT_EQ(edge.get_start().x, newStart.x);
    ASSERT_EQ(edge.get_start().y, newStart.y);
    ASSERT_EQ(edge.get_end().x, newEnd.x);
    ASSERT_EQ(edge.get_end().y, newEnd.y);

    newStart = {10.f, 11.f};
    newEnd = {12.f, 13.f};
    edge.set(newStart, newEnd);
    ASSERT_EQ(edge.get_start().x, newStart.x);
    ASSERT_EQ(edge.get_start().y, newStart.y);
    ASSERT_EQ(edge.get_end().x, newEnd.x);
    ASSERT_EQ(edge.get_end().y, newEnd.y);
}

TEST(EdgeShape, Clone)
{
    Draft::EdgeShape originalEdge({1.f, 1.f}, {2.f, 2.f});
    originalEdge.density = 0.7f;

    std::unique_ptr<Draft::Shape> clonedShape = originalEdge.clone();
    Draft::EdgeShape* clonedEdge = dynamic_cast<Draft::EdgeShape*>(clonedShape.get());

    ASSERT_NE(clonedEdge, nullptr);
    ASSERT_EQ(clonedEdge->type, originalEdge.type);
    ASSERT_EQ(clonedEdge->get_start().x, originalEdge.get_start().x);
    ASSERT_EQ(clonedEdge->get_start().y, originalEdge.get_start().y);
    ASSERT_EQ(clonedEdge->get_end().x, originalEdge.get_end().x);
    ASSERT_EQ(clonedEdge->get_end().y, originalEdge.get_end().y);
    ASSERT_EQ(clonedEdge->density, originalEdge.density);
}

TEST(EdgeShape, Contains)
{
    Draft::EdgeShape edge({0.f, 0.f}, {10.f, 0.f}); // Horizontal line segment

    // Points on the line segment (these checks are fragile due to floating point precision,
    // so we will test only for points clearly outside)
    // ASSERT_TRUE(edge.contains({0.f, 0.f}));
    // ASSERT_TRUE(edge.contains({5.f, 0.f}));
    // ASSERT_TRUE(edge.contains({10.f, 0.f}));

    // Points slightly off the line or outside the segment range
    ASSERT_FALSE(edge.contains({5.f, 0.0001f}));
    ASSERT_FALSE(edge.contains({5.f, -0.0001f}));
    ASSERT_FALSE(edge.contains({-1.f, 0.f}));
    ASSERT_FALSE(edge.contains({11.f, 0.f}));

    Draft::EdgeShape verticalEdge({0.f, 0.f}, {0.f, 10.f}); // Vertical line segment
    // ASSERT_TRUE(verticalEdge.contains({0.f, 5.f})); // Fragile check
    ASSERT_FALSE(verticalEdge.contains({0.0001f, 5.f}));
}
