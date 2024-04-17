#include "draft/phys/shape.hpp"
#include "draft/math/vector2.hpp"
#include "draft/math/vector2_p.hpp"

namespace Draft {
    // Polygon
    PolygonShape::PolygonShape(const PolygonShape& other) : PolygonShape() {
        // Copy data
        for(const auto& v : other.vertices){
            add_vertex(Vector2f(v));
        }
    }

    void PolygonShape::set_as_box(float hw, float hy){
        add_vertex({ -hw, -hy });
        add_vertex({ hw, -hy });
        add_vertex({ hw, hy });
        add_vertex({ -hw, hy });
    }

    size_t PolygonShape::add_vertex(Vector2f vertex){
        vertices.push_back(vertex);
        physVertices.push_back(vector_to_b2(vertex));
        physShape.Set(&physVertices[0], vertices.size());
        return vertices.size() - 1;
    }

    bool PolygonShape::del_vertex(size_t index){
        if(index >= get_vertex_count())
            return false;
        
        vertices.erase(vertices.begin() + index);
        physVertices.erase(physVertices.begin() + index);
        physShape.Set(&physVertices[0], vertices.size());
        return true;
    }
    
    // Circle
    CircleShape::CircleShape(const CircleShape& other): CircleShape(){
        radius = other.radius;
    }
    
    // Edge
    EdgeShape::EdgeShape(const EdgeShape& other): EdgeShape(){
        start = other.start;
        end = other.end;
    }
}