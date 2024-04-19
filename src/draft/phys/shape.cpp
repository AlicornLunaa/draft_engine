#include "draft/phys/shape.hpp"
#include "draft/math/vector2_p.hpp"
#include "draft/phys/shape_p.hpp"
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

    // Conversion functions
    b2PolygonShape shape_to_b2(const PolygonShape& shape){
        // Convert verticees to new point
        size_t count = shape.get_vertex_count();
        b2Vec2 physVerts[count];

        for(size_t i = 0; i < count; i++){
            physVerts[i] = vector_to_b2(shape.get_vertex(i));
        }

        // Create shape;
        b2PolygonShape s;
        s.Set(&physVerts[0], count);
        return s;
    }

    b2CircleShape shape_to_b2(const CircleShape& shape){
        b2CircleShape s;
        s.m_p.Set(shape.position.x, shape.position.y);
        s.m_radius = shape.get_radius();
        return s;
    }

    b2EdgeShape shape_to_b2(const EdgeShape& shape){
        b2EdgeShape s;
        s.SetTwoSided(vector_to_b2(shape.get_start()), vector_to_b2(shape.get_end()));
        return s;
    }
}