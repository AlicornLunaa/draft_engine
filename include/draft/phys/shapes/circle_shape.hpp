#pragma once

#include "draft/phys/shapes/shape.hpp"
#include "draft/math/glm.hpp"

namespace Draft {
    class CircleShape : public Shape {
    private:
        // Variables
        Vector2f position{0, 0};
        float radius = 1.f;

    public:
        // Constructors
        CircleShape() : Shape(ShapeType::CIRCLE) {}

        // Functions
        std::unique_ptr<Shape> clone() const;
        bool contains(const Vector2f& point) const;

        const Vector2f& get_position() const;
        float get_radius() const;

        void set_position(const Vector2f& v);
        void set_radius(float r);
    };
}