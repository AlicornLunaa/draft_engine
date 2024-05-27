#include "draft/phys/shapes/polygon_shape.hpp"
#include "draft/math/vector2_p.hpp"
#include "draft/phys/conversions_p.hpp"

#include "box2d/b2_polygon_shape.h"

namespace Draft {
    // Functions
    std::unique_ptr<Shape> PolygonShape::clone() const {
        auto ptr = std::make_unique<PolygonShape>();
        ptr->isConvex = isConvex;
        ptr->isSensor = isSensor;
        ptr->friction = friction;
        ptr->restitution = restitution;
        ptr->density = density;

        ptr->vertices = vertices;

        return ptr;
    }

    bool PolygonShape::contains(const Vector2f& point) const {
        auto s = shape_to_b2(*this);
        return s.TestPoint(b2Transform(), vector_to_b2(point));
    }

    size_t PolygonShape::add_vertex(Vector2f vertex){
        vertices.push_back(vertex);
        return vertices.size() - 1;
    }

    void PolygonShape::set_as_box(float hw, float hy){
        vertices.clear();
        add_vertex({ -hw, -hy });
        add_vertex({ hw, -hy });
        add_vertex({ hw, hy });
        add_vertex({ -hw, hy });
    }

    void PolygonShape::set_vertex(size_t index, Vector2f vertex){
        vertices[index] = vertex;
    }

    bool PolygonShape::del_vertex(size_t index){
        if(index >= get_vertex_count())
            return false;
        
        vertices.erase(vertices.begin() + index);
        return true;
    }

    const std::vector<Vector2f>& PolygonShape::get_vertices() const {
        return vertices;
    }

    const Vector2f& PolygonShape::get_vertex(size_t index) const {
        return vertices[index];
    }

    size_t PolygonShape::get_vertex_count() const {
        return vertices.size();
    }
}