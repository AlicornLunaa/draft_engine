#include <gtest/gtest.h>
#include "draft/phys/shapes/circle_shape.hpp"

TEST(CircleShape, DefaultConstructor)
{
    Draft::CircleShape circle;
    ASSERT_EQ(circle.type, Draft::ShapeType::CIRCLE);
    ASSERT_EQ(circle.get_position().x, 0.f);
    ASSERT_EQ(circle.get_position().y, 0.f);
    ASSERT_EQ(circle.get_radius(), 1.f);
}

TEST(CircleShape, SetGetPosition)
{
    Draft::CircleShape circle;
    Draft::Vector2f newPos = {5.f, 10.f};
    circle.set_position(newPos);
    ASSERT_EQ(circle.get_position().x, newPos.x);
    ASSERT_EQ(circle.get_position().y, newPos.y);
}

TEST(CircleShape, SetGetRadius)
{
    Draft::CircleShape circle;
    float newRadius = 2.5f;
    circle.set_radius(newRadius);
    ASSERT_EQ(circle.get_radius(), newRadius);
}

TEST(CircleShape, Clone)
{
    Draft::CircleShape originalCircle;
    originalCircle.set_position({1.f, 2.f});
    originalCircle.set_radius(3.f);
    originalCircle.density = 0.5f;

    std::unique_ptr<Draft::Shape> clonedShape = originalCircle.clone();
    Draft::CircleShape* clonedCircle = dynamic_cast<Draft::CircleShape*>(clonedShape.get());

    ASSERT_NE(clonedCircle, nullptr);
    ASSERT_EQ(clonedCircle->type, originalCircle.type);
    ASSERT_EQ(clonedCircle->get_position().x, originalCircle.get_position().x);
    ASSERT_EQ(clonedCircle->get_position().y, originalCircle.get_position().y);
    ASSERT_EQ(clonedCircle->get_radius(), originalCircle.get_radius());
    ASSERT_EQ(clonedCircle->density, originalCircle.density);
}

TEST(CircleShape, Contains)
{
    Draft::CircleShape circle; // Default: center {0,0}, radius 1
    circle.set_position({0.f, 0.f});
    circle.set_radius(1.f);

    ASSERT_TRUE(circle.contains({0.f, 0.f}));      // Center
    ASSERT_TRUE(circle.contains({0.5f, 0.f}));     // Inside
    ASSERT_TRUE(circle.contains({0.f, 0.5f}));     // Inside
    ASSERT_TRUE(circle.contains({0.707f, 0.707f})); // On edge (approx)
    ASSERT_FALSE(circle.contains({1.1f, 0.f}));    // Outside
    ASSERT_FALSE(circle.contains({0.f, -1.1f}));   // Outside

    circle.set_position({5.f, 5.f});
    circle.set_radius(2.f);
    ASSERT_TRUE(circle.contains({5.f, 5.f}));      // New center
    ASSERT_TRUE(circle.contains({6.f, 5.f}));      // Inside new circle
    ASSERT_FALSE(circle.contains({7.1f, 5.f}));    // Outside new circle
}
