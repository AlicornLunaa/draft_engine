#include "draft/phys/shapes/chain_shape.hpp"

namespace Draft {
    // Functions
    std::unique_ptr<Shape> ChainShape::clone() const {
        auto ptr = std::make_unique<ChainShape>();
        ptr->isConvex = isConvex;
        ptr->isSensor = isSensor;
        ptr->friction = friction;
        ptr->restitution = restitution;
        ptr->density = density;
        
        ptr->points = points;

        return ptr;
    }
    
    bool ChainShape::contains(const Vector2f& point) const {
        return false;
    }

    void ChainShape::clear(){
        points.clear();
    }

    void ChainShape::add(const Vector2f& point){
        points.push_back(point);
    }

    void ChainShape::remove(size_t index){
        points.erase(points.begin() + index);
    }

    const std::vector<Vector2f>& ChainShape::get_points() const {
        return points;
    }
}