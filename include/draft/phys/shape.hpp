#pragma once

#include <vector>

#include "draft/math/vector2.hpp"

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

        Vector2f position = {0, 0};
        float rotation = 0.f;

        // Constructors
        Shape(ShapeType type) : type(type) {};
        virtual ~Shape(){};

        // Functions
        virtual bool contains(const Vector2f& point) const = 0;
    };

    class PolygonShape : public Shape {
    private:
        // Variables
        std::vector<Vector2f> vertices;

    public:
        // Constructors
        PolygonShape() : Shape(ShapeType::POLYGON) {}

        // Functions
        bool contains(const Vector2f& point) const;

        void set_as_box(float halfWidth, float halfHeight);
        size_t add_vertex(Vector2f vertex);
        bool del_vertex(size_t index);

        inline const std::vector<Vector2f>& get_vertices() const { return vertices; }
        inline Vector2f get_vertex(size_t index) const { return vertices[index]; }
        inline size_t get_vertex_count() const { return vertices.size(); }
    };

    class CircleShape : public Shape {
    private:
        // Variables
        float radius = 1.f;

    public:
        // Constructors
        CircleShape() : Shape(ShapeType::CIRCLE) {}

        // Functions
        bool contains(const Vector2f& point) const;

        inline float get_radius() const { return radius; }
        inline void set_radius(float r){ radius = r; }
    };

    class EdgeShape : public Shape {
    private:
        // Variables
        Vector2f start;
        Vector2f end;

    public:
        // Constructors
        EdgeShape() : Shape(ShapeType::EDGE) {}

        // Functions
        bool contains(const Vector2f& point) const;

        inline void set_start(Vector2f v){ start = v; }
        inline void set_end(Vector2f v){ end = v; }
        inline const Vector2f& get_start() const { return start; }
        inline const Vector2f& get_end() const { return end; }
    };
}