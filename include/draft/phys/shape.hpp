#pragma once

#include "box2d/b2_math.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_edge_shape.h"
#include "box2d/b2_circle_shape.h"
#include "draft/math/vector2.hpp"

#include <vector>

namespace Draft {
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

        Vector2f position;
        float rotation;

        // Constructors
        Shape(const Shape& other) = default;
        Shape(ShapeType type) : type(type) {};
        virtual ~Shape() = default;

        // Functions
        virtual bool contains(const Vector2f& point) = 0;
        virtual const b2Shape* getPhysShape() = 0;

        // Operators
        virtual operator const b2Shape* () = 0;
    };

    class PolygonShape : public Shape {
    private:
        // Variables
        std::vector<Vector2f> vertices;
        std::vector<b2Vec2> physVertices;
        b2PolygonShape physShape;

    public:
        // Constructors
        PolygonShape() : Shape(ShapeType::POLYGON) {}
        PolygonShape(const PolygonShape& other);
        ~PolygonShape() = default;

        // Functions
        inline bool contains(const Vector2f& point){ return physShape.TestPoint(b2Transform({ position.x, position.y }, b2Rot(rotation)), { point.x, point.y }); };
        inline const b2Shape* getPhysShape(){ return &physShape; }

        void setAsBox(float hw, float hy);
        size_t addVertex(Vector2f vertex);
        bool delVertex(size_t index);

        inline Vector2f getVertex(size_t index){ return vertices[index]; }
        inline size_t getVertexCount(){ return vertices.size(); }

        // Operators
        inline operator const b2Shape* () { return getPhysShape(); };
    };

    class CircleShape : public Shape {
    private:
        // Variables
        float radius = 1.f;
        b2CircleShape physShape;

    public:
        // Constructors
        CircleShape() : Shape(ShapeType::CIRCLE) {}
        CircleShape(const CircleShape& other);
        ~CircleShape() = default;

        // Functions
        inline bool contains(const Vector2f& point){ return physShape.TestPoint(b2Transform({ position.x, position.y }, b2Rot(rotation)), { point.x, point.y }); };
        inline const b2Shape* getPhysShape(){ return &physShape; }

        inline float getRadius(){ return radius; }
        inline void setRadius(float r){ radius = r; }

        // Operators
        inline operator const b2Shape* () { return getPhysShape(); };
    };

    class EdgeShape : public Shape {
    private:
        // Variables
        Vector2f start;
        Vector2f end;
        b2EdgeShape physShape;

    public:
        // Constructors
        EdgeShape() : Shape(ShapeType::EDGE) {}
        EdgeShape(const EdgeShape& other);
        ~EdgeShape() = default;

        // Functions
        inline bool contains(const Vector2f& point){ return false; };
        inline const b2Shape* getPhysShape(){ return &physShape; }

        inline void setStart(Vector2f v){ start = v; }
        inline void setEnd(Vector2f v){ end = v; }

        inline Vector2f getStart(){ return start; }
        inline Vector2f getEnd(){ return end; }

        // Operators
        inline operator const b2Shape* () { return getPhysShape(); };
    };
}