#include "draft/phys/shapes/edge_shape.hpp"
#include "draft/math/vector2_p.hpp"
#include "draft/phys/conversions_p.hpp"

#include "box2d/b2_edge_shape.h"

namespace Draft {
    // Functions
    std::unique_ptr<Shape> EdgeShape::clone() const {
        auto ptr = std::make_unique<EdgeShape>();
        ptr->isConvex = isConvex;
        ptr->isSensor = isSensor;
        ptr->friction = friction;
        ptr->restitution = restitution;
        ptr->density = density;
        
        ptr->start = start;
        ptr->end = end;

        return ptr;
    }
    
    bool EdgeShape::contains(const Vector2f& point) const {
        auto s = shape_to_b2(*this);
        return s.TestPoint(b2Transform(), vector_to_b2(point));
    }

    void EdgeShape::set_start(Vector2f v){
        start = v;
    }

    void EdgeShape::set_end(Vector2f v){
        end = v;
    }

    const Vector2f& EdgeShape::get_start() const {
        return start;
    }

    const Vector2f& EdgeShape::get_end() const {
        return end;
    }
}