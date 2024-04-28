#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/vertex_buffer.hpp"

#include <array>
#include <cstddef>
#include <vector>

namespace Draft {
    class ShapeBatch {
    private:
        // Data structures
        enum class RenderType { FILL, LINE };

        struct ShapeVertex {
            Vector2f position;
            Vector4f color{1, 1, 1, 1};
        };

        static std::array<Vector2f, 4> quadVertices;

        // Variables
        const size_t maxShapes;
        std::vector<std::tuple<RenderType, size_t, size_t>> drawTypes; // Contains render type and the length of its vertices and indices
        std::vector<ShapeVertex> vertices;
        std::vector<int> indices;

        VertexBuffer vertexBuffer;
        size_t dynamicVertexBufLoc;
        size_t dynamicIndexBufLoc;
        Shader& shader;

        Vector4f currentColor{ 1, 1, 1, 1 };
        RenderType currentRenderType = ShapeBatch::RenderType::LINE;
        
        // Private functions
        std::tuple<RenderType, size_t, size_t>& get_current_drawtype_instance();

    public:
        // Constructors
        ShapeBatch(Shader& shader, const size_t maxShapes = 1000);

        // Functions
        inline void set_color(const Vector4f& color){ currentColor = color; }
        void set_render_type(RenderType type);
        void draw_circle(const Vector2f& position, float radius, float rotation, size_t segments = 10);
        void draw_line(const Vector2f& start, const Vector2f& end);
        void flush(); // Send shapes to shader
    };
};