#pragma once

#include <cstddef>
#include <vector>
#include "clydesdale/math/vector2.hpp"

namespace Clydesdale {
    enum class ShapeType { POLYGON, CIRCLE, EDGE };

    class Shape {
    public:
        // Public variables
        const ShapeType type;
        bool isConvex = true;
        bool isSensor = false;
        float friction = 0.2f;
        float restitution = 0.2f;
        float density = 1.0f;

        // Constructors
        Shape(const Shape& other) = default;
        Shape(ShapeType type) : type(type) {};
        ~Shape();

        // Functions
        virtual void getShape() = 0; // TODO: Implement abstract shape
    };

    class PolygonShape : public Shape {
    private:
        // Variables
        std::vector<Clydesdale::Vector2f> vertices;
        std::vector<size_t> indices;

        // Private functions
        void wrap(); // Simplifies shape with giftwrapping algorithm
        void simplify(); // Removes useless vertices

    public:
        // Constructors
        PolygonShape() : Shape(ShapeType::POLYGON) {}
        PolygonShape(const PolygonShape& other);
        ~PolygonShape();

        // Functions
        void getShape();

        size_t addVertex(Clydesdale::Vector2f vertex);
        bool delVertex(size_t index);

        inline Clydesdale::Vector2f getVertex(size_t index){ return vertices[index]; }
        inline size_t getVertexCount(){ return vertices.size(); }
    };

    class CircleShape : public Shape {
    private:
        // Variables
        Clydesdale::Vector2f position;
        float radius = 1.f;

    public:
        // Constructors
        CircleShape() : Shape(ShapeType::CIRCLE) {}
        CircleShape(const CircleShape& other);
        ~CircleShape();

        // Functions
        void getShape();

        inline void setPosition(Clydesdale::Vector2f pos){ position = pos; }
        inline void setRadius(float rad){ radius = rad; }

        inline Clydesdale::Vector2f getPosition(){ return position; }
        inline float getRadius(){ return radius; }
    };

    class EdgeShape : public Shape {
    private:
        // Variables
        Clydesdale::Vector2f start;
        Clydesdale::Vector2f end;

    public:
        // Constructors
        EdgeShape() : Shape(ShapeType::EDGE) {}
        EdgeShape(const EdgeShape& other);
        ~EdgeShape();

        // Functions
        void getShape();

        inline void setStart(Clydesdale::Vector2f v){ start = v; }
        inline void setEnd(Clydesdale::Vector2f v){ end = v; }

        inline Clydesdale::Vector2f getStart(){ return start; }
        inline Clydesdale::Vector2f getEnd(){ return end; }
    };
}