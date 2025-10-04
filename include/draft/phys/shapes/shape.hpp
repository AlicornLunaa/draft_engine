#pragma once

#include <memory>

#include "draft/math/glm.hpp"

namespace Draft {
    enum class ShapeType { POLYGON, CIRCLE, EDGE, CHAIN };

    class Shape {
    public:
        // Public variables
        ShapeType type;
        bool isConvex = true;
        bool isSensor = false;
        float friction = 0.2f;
        float restitution = 0.2f;
        float density = 1.0f;

        // Constructors
        Shape(ShapeType type) : type(type) {};
        virtual ~Shape() = default;

        // Functions
        virtual std::unique_ptr<Shape> clone() const = 0;
        virtual bool contains(const Vector2f& point) const = 0;
    };
}