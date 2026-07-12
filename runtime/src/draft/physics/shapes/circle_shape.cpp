#include "draft/physics/shapes/circle_shape.hpp"

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
        // Direct port of b2CircleShape::TestPoint
        return Math::distance2(point, position) <= radius * radius;
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
