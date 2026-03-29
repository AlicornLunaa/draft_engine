#include <gtest/gtest.h>
#include "draft/phys/shapes/shape.hpp"

TEST(Shape, ShapeTypeEnum)
{
    ASSERT_EQ(static_cast<int>(Draft::ShapeType::POLYGON), 0);
    ASSERT_EQ(static_cast<int>(Draft::ShapeType::CIRCLE), 1);
    ASSERT_EQ(static_cast<int>(Draft::ShapeType::EDGE), 2);
    ASSERT_EQ(static_cast<int>(Draft::ShapeType::CHAIN), 3);
}

// Dummy concrete class because Shape is abstract
class ConcreteShape : public Draft::Shape {
public:
    ConcreteShape() : Draft::Shape(Draft::ShapeType::POLYGON) {}
    std::unique_ptr<Draft::Shape> clone() const override {
        return std::make_unique<ConcreteShape>(*this);
    }
    bool contains(const Draft::Vector2f& point) const override {
        return false;
    }
};

TEST(Shape, DefaultProperties)
{
    ConcreteShape shape; // Using a concrete derived class to access protected constructor
    ASSERT_TRUE(shape.isConvex);
    ASSERT_FALSE(shape.isSensor);
    ASSERT_EQ(shape.friction, 0.2f);
    ASSERT_EQ(shape.restitution, 0.2f);
    ASSERT_EQ(shape.density, 1.0f);
}
