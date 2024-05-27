#pragma once

#include "draft/phys/shapes/shape.hpp"
#include "draft/math/glm.hpp"

#include <vector>

namespace Draft {
    class PolygonShape : public Shape {
    private:
        // Variables
        std::vector<Vector2f> vertices;

    public:
        // Constructors
        PolygonShape() : Shape(ShapeType::POLYGON) {}

        // Functions
        std::unique_ptr<Shape> clone() const;
        bool contains(const Vector2f& point) const;

        size_t add_vertex(Vector2f vertex);
        void set_as_box(float halfWidth, float halfHeight);
        void set_vertex(size_t index, Vector2f vertex);
        bool del_vertex(size_t index);

        const std::vector<Vector2f>& get_vertices() const;
        const Vector2f& get_vertex(size_t index) const;
        size_t get_vertex_count() const;
    };
}