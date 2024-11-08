#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/vertex_array.hpp"
#include "draft/rendering/batching/batch.hpp"
#include "draft/rendering/vertex_array.hpp"
#include "draft/util/asset_manager/asset_manager.hpp"

#include <array>
#include <cstddef>
#include <vector>

namespace Draft {
    class ShapeBatch : public Batch {
    public:
        static constexpr size_t MAX_SHAPES_TO_RENDER = 1024;
        enum class RenderType { FILL, LINE };

    private:
        // Data structures
        struct Point {
            Vector2f position;
            Vector4f color{1, 1, 1, 1};
        };

        // Static data
        const std::vector<Vector2f> TRI_VERTICES = {
            Vector2f(0, 0), // Top
            Vector2f(1, 0), // Bottom-right
            Vector2f(0, 1) // Bottom-left
        };
        const std::vector<Vector2f> QUAD_VERTICES = {
            Vector2f(0, 0), // Top-left
            Vector2f(1, 0), // Top-right
            Vector2f(1, 1), // Bottom-right
            Vector2f(0, 1) // Bottom-left
        };
        const std::vector<int> QUAD_INDICES = { 0, 1, 2, 2, 3, 0 };

        // Variables
        VertexArray vertexArray;
        std::vector<Point> points;
        Vector4f currentColor{ 1, 1, 1, 1 };
        RenderType currentRenderType = ShapeBatch::RenderType::LINE;

    public:
        // Constructors
        ShapeBatch(Resource<Shader> shader = Assets::manager.get<Shader>("assets/shaders/shapes", true));
        virtual ~ShapeBatch() = default;

        // Functions
        inline void set_color(const Vector4f& color){ currentColor = color; }
        void set_render_type(RenderType type);

        inline const Vector4f& get_color() const { return currentColor; }
        inline const RenderType& get_render_type() const { return currentRenderType; }
        
        void draw_polygon(const std::vector<Vector2f>& polygonVertices);
        void draw_rect(const Vector2f& position, const Vector2f& size, float rotation);
        void draw_triangle(const Vector2f& position, const Vector2f& size, float rotation);
        void draw_triangle(const std::array<Vector2f, 3>& positions);
        void draw_circle(const Vector2f& position, float radius, float rotation, size_t segments = 10);
        void draw_line(const Vector2f& start, const Vector2f& end);
        void draw_rect_line(const Vector2f& start, const Vector2f& end, float width = 1.f);
        void draw_arrow(const Vector2f& head, const Vector2f& tail);

        virtual void begin();
        virtual void flush(); // Send shapes to shader
        virtual void end();
    };
};