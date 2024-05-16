#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/camera.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/vertex_buffer.hpp"
#include "draft/util/asset_manager.hpp"

#include <array>
#include <cstddef>
#include <queue>
#include <vector>

namespace Draft {
    class ShapeBatch {
    public:
        enum class RenderType { FILL, LINE };

    private:
        // Data structures
        struct ShapeVertex {
            Vector2f position;
            Vector4f color{1, 1, 1, 1};
        };

        static std::array<Vector2f, 4> quadVertices;

        // Variables
        const size_t maxShapes;
        std::queue<std::tuple<RenderType, size_t, size_t>> renderTypes; // Contains render type and the length of its vertices and indices
        std::vector<ShapeVertex> vertices;
        std::vector<int> indices;

        std::shared_ptr<Shader> shader;
        VertexBuffer vertexBuffer;
        size_t dynamicVertexBufLoc;
        size_t dynamicIndexBufLoc;

        Vector4f currentColor{ 1, 1, 1, 1 };
        RenderType currentRenderType = ShapeBatch::RenderType::LINE;
        
        // Private functions
        std::tuple<RenderType, size_t, size_t>& get_current_render_type_instance();

    public:
        // Constructors
        ShapeBatch(std::shared_ptr<Shader> shader = Assets::manager.get<Shader>("assets/shaders/shapes", true), const size_t maxShapes = 1000);

        // Functions
        inline void set_color(const Vector4f& color){ currentColor = color; }
        void set_render_type(RenderType type);
        void draw_polygon(const std::vector<Vector2f>& polygonVertices);
        void draw_rect(const Vector2f& position, const Vector2f& size, float rotation);
        void draw_circle(const Vector2f& position, float radius, float rotation, size_t segments = 10);
        void draw_line(const Vector2f& start, const Vector2f& end);
        void draw_rect_line(const Vector2f& start, const Vector2f& end, float width = 1.f);
        void draw_arrow(const Vector2f& head, const Vector2f& tail);
        void flush(const RenderWindow& window, const Camera* camera = nullptr); // Send shapes to shader
    };
};