#pragma once

#include "draft/phys/shapes/shape.hpp"
#include "draft/math/glm.hpp"
#include <vector>

namespace Draft {
    class ChainShape : public Shape {
    public:
        // Enumerators
        enum ChainType { LOOP, CHAIN };

        // Constructors
        ChainShape(ChainType type) : Shape(ShapeType::CHAIN), m_chainType(type) {}

        // Functions
        std::unique_ptr<Shape> clone() const;
        bool contains(const Vector2f& point) const;

        void set_previous(const Vector2f& point);
        void set_next(const Vector2f& point);

        void clear();
        void add(const Vector2f& point);
        void remove(size_t index);

        const std::vector<Vector2f>& get_points() const;
        const Vector2f& get_previous() const;
        const Vector2f& get_next() const;
        ChainType get_chain_type() const;

    private:
        // Variables
        std::vector<Vector2f> m_points;
        Vector2f m_prev, m_next; // Not used unless CHAIN type
        ChainType m_chainType;
    };
}