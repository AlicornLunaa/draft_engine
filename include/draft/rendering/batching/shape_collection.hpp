#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/batching/collection.hpp"
#include "draft/rendering/batching/draw_command.hpp"
#include "draft/rendering/batching/shape_point.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/vertex_array.hpp"
#include <queue>

namespace Draft {
    class ShapeCollection : public Collection {
    public:
        // Constructors
        ShapeCollection(Resource<Shader> shader = s_defaultShader);
        virtual ~ShapeCollection() = default;

        // Functions
        inline void set_color(const Vector4f& color){ m_currentColor = color; }
        void set_z_layer(float depth);
        void set_render_type(ShapeRenderType type);
        void set_shader(Resource<Shader> shader);
        
        inline const Vector4f& get_color() const { return m_currentColor; }
        inline float get_z_layer() const { return m_zLayer; }
        inline const ShapeRenderType& get_render_type() const { return m_currentRenderType; }
        inline const Shader& get_shader() const { return m_shader; }
        
        void draw_polygon(const std::vector<Vector2f>& polygonVertices);
        void draw_rect(const Vector2f& position, const Vector2f& size, float rotation);
        void draw_triangle(const Vector2f& position, const Vector2f& size, float rotation);
        void draw_triangle(const std::array<Vector2f, 3>& positions);
        void draw_circle(const Vector2f& position, float radius, float rotation, size_t segments = 10);
        void draw_line(const Vector2f& start, const Vector2f& end);
        void draw_rect_line(const Vector2f& start, const Vector2f& end, float width = 1.f);
        void draw_dotted_line(Vector2f start, const Vector2f& end, float width = 1.f, float spacing = 1.f);
        void draw_arrow(const Vector2f& head, const Vector2f& tail, float arrowScale = 1.f);

        virtual void flush() override; // Send shapes to shader

    private:
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

        static StaticResource<Shader> s_defaultShader;

        // Variables
        VertexArray m_vertexArray;

        std::queue<ShapeDrawCommand> m_drawCommands;
        Vector4f m_currentColor{ 1, 1, 1, 1 };
        ShapeRenderType m_currentRenderType = ShapeRenderType::LINE;
        Resource<Shader> m_shader;
        float m_zLayer = 0.f;

        bool m_commandDirty = true;
        bool m_shaderDirty = true;
        bool m_layerDirty = true;

        // Private functions
        void new_command();
        bool needs_new_command() const;
    };
};