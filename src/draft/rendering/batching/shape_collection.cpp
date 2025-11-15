#include "draft/math/glm.hpp"
#include "draft/rendering/batching/shape_collection.hpp"
#include "draft/rendering/batching/shape_point.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/vertex_array.hpp"
#include "draft/util/asset_manager/resource.hpp"
#include "draft/util/file_handle.hpp"
#include "draft/util/logger.hpp"
#include "glad/gl.h"
#include "glm/geometric.hpp"
#include <cassert>
#include <tracy/Tracy.hpp>

using namespace std;

namespace Draft {
    // Static data
    StaticResource<Shader> ShapeCollection::s_defaultShader = {FileHandle("assets/shaders/shapes")};

    // Private functions
    void ShapeCollection::new_command(){
        if(m_drawCommands.empty()){
            p_matricesDirty = true;
            m_shaderDirty = true;
            m_layerDirty = true;
        }

        m_drawCommands.push({
            {},
            get_proj_matrix(),
            get_trans_matrix(),
            m_currentRenderType,
            m_shader,
            m_zLayer,
            p_matricesDirty,
            m_shaderDirty,
            m_layerDirty
        });

        p_matricesDirty = false;
        m_commandDirty = false;
        m_shaderDirty = false;
        m_layerDirty = false;
    }

    // Constructor
    ShapeCollection::ShapeCollection(Resource<Shader> shader) : Collection(), m_shader(shader) {
        // Setup data buffers
        m_vertexArray.create({
            StaticBuffer::create<ShapePoint>({
                BufferAttribute{0, GL_FLOAT, 2, sizeof(ShapePoint), 0, false},
                BufferAttribute{1, GL_FLOAT, 4, sizeof(ShapePoint), offsetof(ShapePoint, color), false}
            }, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW)
        });
    }

    // Functions
    void ShapeCollection::set_render_type(ShapeRenderType type){
        if(m_currentRenderType != type)
            m_commandDirty = true;

        m_currentRenderType = type;
    }

    void ShapeCollection::set_z_layer(float depth){
        if(m_zLayer != depth){
            m_commandDirty = true;
            m_layerDirty = true;
        }
        
        m_zLayer = depth;
    }

    void ShapeCollection::set_shader(Resource<Shader> shader){
        if(shader != this->m_shader){
            m_commandDirty = true;
            m_shaderDirty = true;
            p_matricesDirty = true;
        }
        
        m_shader = shader;
    }

    void ShapeCollection::draw_polygon(const std::vector<Vector2f>& polygonVertices){
        // Profiling
        ZoneScopedN("shape_batch_polygon");

        if(m_commandDirty)
            new_command();
        
        auto& points = m_drawCommands.back().points;

        // Generate and add vertices
        for(size_t i = 0; i < polygonVertices.size(); i++){
            points.push_back({ polygonVertices[i], m_currentColor });
            points.push_back({ polygonVertices[(i + 1) % polygonVertices.size()], m_currentColor });
        }
    }

    void ShapeCollection::draw_rect(const Vector2f& position, const Vector2f& size, float rotation){
        // Profiling
        ZoneScopedN("shape_batch_rect");

        if(m_commandDirty)
            new_command();

        // Generate and add vertices
        if(m_currentRenderType == ShapeRenderType::FILL){
            // Two triangles to fill
            auto& points = m_drawCommands.back().points;

            for(int index : QUAD_INDICES){
                points.push_back({QUAD_VERTICES[index] * size + position, m_currentColor});
            }
        } else {
            // Lines
            auto& points = m_drawCommands.back().points;

            for(size_t i = 0; i < QUAD_VERTICES.size(); i++){
                points.push_back({QUAD_VERTICES[i] * size + position, m_currentColor});
                points.push_back({QUAD_VERTICES[(i + 1) % QUAD_VERTICES.size()] * size + position, m_currentColor});
            }
        }
    }

    void ShapeCollection::draw_triangle(const Vector2f& position, const Vector2f& size, float rotation){
        // Profiling
        ZoneScopedN("shape_batch_triangle");

        if(m_commandDirty)
            new_command();

        // Connect all indices, depending on filled or lines
        if(m_currentRenderType == ShapeRenderType::FILL){
            // One triangles to fill
            auto& points = m_drawCommands.back().points;

            for(const Vector2f& v : TRI_VERTICES){
                points.push_back({Math::rotate(v * size, rotation) + position, m_currentColor});
            }
        } else {
            // Lines
            auto& points = m_drawCommands.back().points;

            for(size_t i = 0; i < TRI_VERTICES.size(); i++){
                points.push_back({Math::rotate(TRI_VERTICES[i] * size, rotation) + position, m_currentColor});
                points.push_back({Math::rotate(TRI_VERTICES[(i + 1) % TRI_VERTICES.size()] * size, rotation) + position, m_currentColor});
            }
        }
    }

    void ShapeCollection::draw_triangle(const std::array<Vector2f, 3>& positions){
        // Profiling
        ZoneScopedN("shape_batch_triangle");

        if(m_commandDirty)
            new_command();

        // Connect all indices, depending on filled or lines
        if(m_currentRenderType == ShapeRenderType::FILL){
            // Two triangles to fill
            auto& points = m_drawCommands.back().points;

            points.push_back({ positions[0], m_currentColor });
            points.push_back({ positions[1], m_currentColor });
            points.push_back({ positions[2], m_currentColor });
        } else {
            // Lines
            auto& points = m_drawCommands.back().points;

            for(size_t i = 0; i < positions.size(); i++){
                points.push_back({ positions[i], m_currentColor });
                points.push_back({ positions[(i + 1) % positions.size()], m_currentColor });
            }
        }
    }

    void ShapeCollection::draw_circle(const Vector2f& position, float radius, float rotation, size_t segments){
        // Profiling
        ZoneScopedN("shape_batch_circle");

        if(m_commandDirty)
            new_command();

        // Generate and add vertices
        float pointsEveryRadian = 2*3.14f / segments;

        // Connect all indices, depending on filled or lines
        if(m_currentRenderType == ShapeRenderType::FILL){
            auto& points = m_drawCommands.back().points;

            for(size_t i = 0; i < segments; i++){
                float radians1 = i * pointsEveryRadian;
                float radians2 = (i + 1) % segments * pointsEveryRadian;
                Vector2f coords1 = Vector2f(std::cos(radians1 + rotation), std::sin(radians1 + rotation)) * radius + position;
                Vector2f coords2 = Vector2f(std::cos(radians2 + rotation), std::sin(radians2 + rotation)) * radius + position;
                points.push_back({coords1, m_currentColor});
                points.push_back({coords2, m_currentColor});
                points.push_back({position, m_currentColor});
            }
        } else {
            // Lines
            auto& points = m_drawCommands.back().points;

            for(size_t i = 0; i < segments; i++){
                float radians1 = i * pointsEveryRadian;
                float radians2 = (i + 1) % segments * pointsEveryRadian;
                Vector2f coords1 = Vector2f(std::cos(radians1 + rotation), std::sin(radians1 + rotation)) * radius + position;
                Vector2f coords2 = Vector2f(std::cos(radians2 + rotation), std::sin(radians2 + rotation)) * radius + position;
                points.push_back({coords1, m_currentColor});
                points.push_back({coords2, m_currentColor});
            }

            points.push_back({position, m_currentColor});
            points.push_back({Vector2f(std::cos(rotation), std::sin(rotation)) * radius + position, m_currentColor});
        }
    }

    void ShapeCollection::draw_line(const Vector2f& start, const Vector2f& end){
        // Profiling
        ZoneScopedN("shape_batch_line");

        // Lines can only be GL_LINES
        if(m_currentRenderType != ShapeRenderType::LINE){
            Logger::println(Level::WARNING, "Shape Batch", "draw_line(const Vector2f&, const Vector2f&) may only be called with LINE render type.\n\tIt was set automatically, but you should do it manually.");
            set_render_type(ShapeRenderType::LINE);
        }

        if(m_commandDirty)
            new_command();

        auto& points = m_drawCommands.back().points;

        // Generate and add vertices
        points.push_back({ start, m_currentColor });
        points.push_back({ end, m_currentColor });
    }

    void ShapeCollection::draw_rect_line(const Vector2f& start, const Vector2f& end, float width){
        // Profiling
        ZoneScopedN("shape_batch_rect_line");

        if(m_commandDirty)
            new_command();

        // Generate and add vertices
        float radians = std::atan2(start.y - end.y, start.x - end.x);
        Vector2f right = Math::rotate(Vector2f(0, 1), radians);
        right *= 0.02f * width;

        // Connect all indices, depending on filled or lines
        if(m_currentRenderType == ShapeRenderType::FILL){
            // Two triangles to fill
            auto& points = m_drawCommands.back().points;

            points.push_back({ start - right, m_currentColor }); // 1
            points.push_back({ start + right, m_currentColor }); // 0
            points.push_back({ end + right, m_currentColor }); // 3
            points.push_back({ start - right, m_currentColor }); // 1
            points.push_back({ end + right, m_currentColor }); // 3
            points.push_back({ end - right, m_currentColor }); // 2
        } else {
            // Lines
            auto& points = m_drawCommands.back().points;

            points.push_back({ start + right, m_currentColor }); // 0
            points.push_back({ start - right, m_currentColor }); // 1
            points.push_back({ start - right, m_currentColor }); // 1
            points.push_back({ end - right, m_currentColor }); // 2
            points.push_back({ end - right, m_currentColor }); // 2
            points.push_back({ end + right, m_currentColor }); // 3
            points.push_back({ end + right, m_currentColor }); // 3
            points.push_back({ start + right, m_currentColor }); // 0
        }
    }

    void ShapeCollection::draw_dotted_line(Vector2f start, const Vector2f& end, float width, float spacing){
        // Divide the length into segments
        uint segmentCount = Math::distance(start, end) / spacing;
        Vector2f r = (end - start) / (float)segmentCount; // Takes the length of the line and divides it by the spacing in pixels
        
        // Draw each segment, ignoring every other
        for(uint i = 0; i < segmentCount; i++){
            if(i % 2 == 0)
                draw_rect_line(start, start + r, width);
            
            start += r;
        }
    }

    void ShapeCollection::draw_arrow(const Vector2f& head, const Vector2f& tail, float arrowScale){
        // Profiling
        ZoneScopedN("shape_batch_arrow");

        // Lines can only be GL_LINES
        if(m_currentRenderType != ShapeRenderType::LINE){
            Logger::println(Level::WARNING, "Shape Batch", "draw_arrow(const Vector2f&, const Vector2f&) may only be called with LINE render type.\n\tIt was set automatically, but you should do it manually.");
            set_render_type(ShapeRenderType::LINE);
        }

        if(m_commandDirty)
            new_command();

        // Get the size vertices for the arrow head
        float radians = std::atan2(head.y - tail.y, head.x - tail.x);
        Vector2f left = Math::rotate(Vector2f(0, 1), radians + 3.141592654f/3.f) * 0.08f * arrowScale;
        Vector2f right = Math::rotate(Vector2f(0, 1), radians + 2*3.141592654f/3.f) * 0.08f * arrowScale;

        // Generate and add vertices
        auto& points = m_drawCommands.back().points;

        points.push_back({ head, m_currentColor }); // 0
        points.push_back({ head + Vector2f(left.x, left.y), m_currentColor }); // 1
        points.push_back({ head, m_currentColor }); // 0
        points.push_back({ head + Vector2f(right.x, right.y), m_currentColor }); // 2
        points.push_back({ head, m_currentColor }); // 0
        points.push_back({ tail, m_currentColor }); // 3
    }

    void ShapeCollection::flush(){
        // Profiling
        ZoneScopedN("shape_batch_flush");

        // Attach the VAO for rendering
        m_vertexArray.bind();

        while(!m_drawCommands.empty()){
            const ShapeDrawCommand& command = m_drawCommands.front();
            auto& points = command.points;

            // Set shader state
            if(command.shaderDirty){
                command.shader->bind();
            }

            if(command.matricesDirty){
                command.shader->set_uniform("view", command.transformMatrix);
                command.shader->set_uniform("projection", command.projectionMatrix);
            }

            if(command.layerDirty){
                command.shader->set_uniform("zLayer", command.zLayer);
            }

            // Draws all the shapes to opengl
            if(points.empty()){
                m_drawCommands.pop();
                continue;
            }
            
            // Render VBO
            m_vertexArray.set_data(0, points);
            glDrawArrays((command.type == ShapeRenderType::LINE) ? GL_LINES : GL_TRIANGLES, 0, points.size());

            m_drawCommands.pop();
        }

        m_vertexArray.unbind();
        new_command();
    }
};