#include "draft/phys/shape.hpp"
#include "draft/math/vector2_p.hpp"
#include "draft/phys/conversions_p.hpp"
#include "draft/math/vector2.hpp"

#include "box2d/b2_math.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_circle_shape.h"
#include "box2d/b2_edge_shape.h"

namespace Draft {
    // Polygon
    bool PolygonShape::contains(const Vector2f& point) const {
        auto s = shape_to_b2(*this);
        auto t = b2Transform(vector_to_b2(position), b2Rot(rotation));
        return s.TestPoint(t, vector_to_b2(point));
    }

    void PolygonShape::set_as_box(float hw, float hy){
        vertices.clear();
        add_vertex({ -hw, -hy });
        add_vertex({ hw, -hy });
        add_vertex({ hw, hy });
        add_vertex({ -hw, hy });
    }

    size_t PolygonShape::add_vertex(Vector2f vertex){
        vertices.push_back(vertex);
        return vertices.size() - 1;
    }

    bool PolygonShape::del_vertex(size_t index){
        if(index >= get_vertex_count())
            return false;
        
        vertices.erase(vertices.begin() + index);
        return true;
    }
    
    // Circle
    bool CircleShape::contains(const Vector2f& point) const {
        auto s = shape_to_b2(*this);
        auto t = b2Transform(vector_to_b2(position), b2Rot(rotation));
        return s.TestPoint(t, vector_to_b2(point));
    }
    
    // Edge
    bool EdgeShape::contains(const Vector2f& point) const {
        auto s = shape_to_b2(*this);
        auto t = b2Transform(vector_to_b2(position), b2Rot(rotation));
        return s.TestPoint(t, vector_to_b2(point));
    }
}