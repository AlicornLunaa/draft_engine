#include "draft/physics/shapes/edge_shape.hpp"

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
        // edges are 1d objects
        return false;
    }

    void EdgeShape::set(const Vector2f& s, const Vector2f& e){
        set_start(s);
        set_end(e);
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
