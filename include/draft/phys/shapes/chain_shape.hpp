#pragma once

#include "draft/phys/shapes/shape.hpp"
#include "draft/math/glm.hpp"
#include <vector>

namespace Draft {
    class ChainShape : public Shape {
    private:
        // Variables
        std::vector<Vector2f> points;

    public:
        // Constructors
        ChainShape() : Shape(ShapeType::CHAIN) {}

        // Functions
        std::unique_ptr<Shape> clone() const;
        bool contains(const Vector2f& point) const;

        void clear();
        void add(const Vector2f& point);
        void remove(size_t index);
        const std::vector<Vector2f>& get_points() const;
    };
}