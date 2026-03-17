#include "draft/phys/shapes/chain_shape.hpp"

namespace Draft {
    // Functions
    std::unique_ptr<Shape> ChainShape::clone() const {
        auto ptr = std::make_unique<ChainShape>(m_chainType);
        ptr->isConvex = isConvex;
        ptr->isSensor = isSensor;
        ptr->friction = friction;
        ptr->restitution = restitution;
        ptr->density = density;
        
        ptr->m_points = m_points;

        return ptr;
    }

    void ChainShape::set_previous(const Vector2f& point){
        m_prev = point;
    }

    void ChainShape::set_next(const Vector2f& point){
        m_next = point;
    }
    
    bool ChainShape::contains(const Vector2f& point) const {
        return false;
    }

    void ChainShape::clear(){
        m_points.clear();
    }

    void ChainShape::add(const Vector2f& point){
        m_points.push_back(point);
    }

    void ChainShape::remove(size_t index){
        m_points.erase(m_points.begin() + index);
    }

    const std::vector<Vector2f>& ChainShape::get_points() const {
        return m_points;
    }

    const Vector2f& ChainShape::get_previous() const {
        return m_prev;
    }

    const Vector2f& ChainShape::get_next() const {
        return m_next;
    }

    ChainShape::ChainType ChainShape::get_chain_type() const {
        return m_chainType;
    }
}