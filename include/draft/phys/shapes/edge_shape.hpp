#pragma once

#include "draft/phys/shapes/shape.hpp"
#include "draft/math/glm.hpp"

namespace Draft {
    class EdgeShape : public Shape {
    private:
        // Variables
        Vector2f start;
        Vector2f end;

    public:
        // Constructors
        EdgeShape() : Shape(ShapeType::EDGE) {}
        
        EdgeShape(const Vector2f& start, const Vector2f& end) : EdgeShape() {
            this->start = start;
            this->end = end;
        }

        // Functions
        std::unique_ptr<Shape> clone() const;
        bool contains(const Vector2f& point) const;

        void set(const Vector2f& s, const Vector2f& e);
        void set_start(Vector2f v);
        void set_end(Vector2f v);
        const Vector2f& get_start() const;
        const Vector2f& get_end() const;
    };
}