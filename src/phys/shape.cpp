#include "draft/phys/shape.hpp"
#include "draft/math/vector2.hpp"

namespace Draft {
    // Polygon
    PolygonShape::PolygonShape(const PolygonShape& other) : PolygonShape() {
        // Copy data
        for(const auto& v : other.vertices){
            addVertex(Vector2f(v));
        }
    }

    void PolygonShape::setAsBox(float hw, float hy){
        addVertex({ -hw, -hy });
        addVertex({ hw, -hy });
        addVertex({ hw, hy });
        addVertex({ -hw, hy });
    }

    size_t PolygonShape::addVertex(Vector2f vertex){
        vertices.push_back(vertex);
        physVertices.push_back(vertex);
        physShape.Set(&physVertices[0], vertices.size());
        return vertices.size() - 1;
    }

    bool PolygonShape::delVertex(size_t index){
        if(index >= getVertexCount())
            return false;
        
        vertices.erase(vertices.begin() + index);
        physVertices.erase(physVertices.begin() + index);
        return physShape.Set(&physVertices[0], vertices.size());;
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