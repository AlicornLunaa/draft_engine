#include "draft/phys/shapes/circle_shape.hpp"
#include "draft/math/vector2_p.hpp"
#include "draft/phys/conversions_p.hpp"

#include "box2d/b2_circle_shape.h"

namespace Draft {
    // Functions
    std::unique_ptr<Shape> CircleShape::clone() const {
        auto ptr = std::make_unique<CircleShape>();
        
        ptr->isConvex = isConvex;
        ptr->isSensor = isSensor;
        ptr->friction = friction;
        ptr->restitution = restitution;
        ptr->density = density;

        ptr->position = position;
        ptr->radius = radius;

        return ptr;
    }
    
    bool CircleShape::contains(const Vector2f& point) const {
        auto s = shape_to_b2(*this);
        return s.TestPoint(b2Transform(), vector_to_b2(point));
    }

    const Vector2f& CircleShape::get_position() const {
        return position;
    }

    float CircleShape::get_radius() const {
        return radius;
    }

    void CircleShape::set_position(const Vector2f& v){
        position = v;
    }

    void CircleShape::set_radius(float r){
        radius = r;
    }
}