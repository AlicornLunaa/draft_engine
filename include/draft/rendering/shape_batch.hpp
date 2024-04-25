#pragma once

#include "draft/math/glm.hpp"

#include <array>
#include <queue>

namespace Draft {
    class ShapeBatch {
    private:
        // Data structures
        enum class Primitive { QUAD, CIRCLE, LINE };

        struct Shape {
            std::vector<Vector2f> vertices;
            std::vector<int> indices;
            Vector2f position = {0, 0};
            Vector2f size = {0, 0};
            Vector3f color = { 1, 1, 1 };
            float rotation = 0.f;
            Primitive type;
        };

        static std::array<Vector2f, 4> quadVertices;

        // Variables
        Vector3f currentColor{ 1, 1, 1 };
        std::queue<Shape> shapeQueue;
        
        // Private functions
        void generate_circle_vertices(std::vector<Vector2f>& vertices, std::vector<int>& indices, size_t segments);
        Matrix4 generate_transform_matrix(const Shape& quad) const;

    public:
        // Constructors
        ShapeBatch();

        // Functions
        inline void set_color(const Vector3f& color){ currentColor = color; }
        void draw_circle(const Vector2f& position, float radius, size_t segments = 10);
        void flush(); // Send quads to shader
    };
};