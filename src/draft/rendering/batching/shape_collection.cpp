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
    StaticResource<Shader> ShapeCollection::defaultShader = {FileHandle("assets/shaders/shapes")};

    // Private functions
    void ShapeCollection::new_command(){
        m_drawCommands.push({
            {},
            get_proj_matrix(),
            get_trans_matrix(),
            currentRenderType,
            shader,
            zLayer
        });
    }

    void ShapeCollection::new_command_if_overflowing(uint count){
        // This function will flush the batch if it is about to overflow, i.e. go higher than MAX_POINTS_PER_PASS
        // This is due to the fact if it reaches the end and cant fit all its vertices in, itll go missing
        auto& points = m_drawCommands.back().points;

        if(points.size() + count >= MAX_POINTS_PER_PASS){
            new_command();
        }
    }

    // Constructor
    ShapeCollection::ShapeCollection(Resource<Shader> shader) : Collection(), shader(shader) {
        // Setup data buffers
        vertexArray.create({
            DynamicBuffer::create<ShapePoint>(MAX_POINTS_PER_PASS, {
                BufferAttribute{0, GL_FLOAT, 2, sizeof(ShapePoint), 0, false},
                BufferAttribute{1, GL_FLOAT, 4, sizeof(ShapePoint), offsetof(ShapePoint, color), false}
            })
        });

        new_command();
    }

    // Functions
    void ShapeCollection::set_render_type(ShapeRenderType type){
        if(currentRenderType != type)
            m_commandDirty = true;

        currentRenderType = type;
    }

    void ShapeCollection::set_z_layer(float depth){
        if(zLayer != depth)
            m_commandDirty = true;

        zLayer = depth;
    }

    void ShapeCollection::set_shader(Resource<Shader> shader){
        if(shader != this->shader)
            m_commandDirty = true;

        this->shader = shader;
    }

    void ShapeCollection::draw_polygon(const std::vector<Vector2f>& polygonVertices){
        // Profiling
        ZoneScopedN("shape_batch_polygon");

        if(m_commandDirty)
            new_command();
        
        assert(polygonVertices.size() < MAX_POINTS_PER_PASS && "Polygon cannot be bigger than max batchable points");
        new_command_if_overflowing(polygonVertices.size() * 2);
        auto& points = m_drawCommands.back().points;

        // Generate and add vertices
        for(size_t i = 0; i < polygonVertices.size(); i++){
            points.push_back({ polygonVertices[i], currentColor });
            points.push_back({ polygonVertices[(i + 1) % polygonVertices.size()], currentColor });
        }
    }

    void ShapeCollection::draw_rect(const Vector2f& position, const Vector2f& size, float rotation){
        // Profiling
        ZoneScopedN("shape_batch_rect");

        if(m_commandDirty)
            new_command();

        // Generate and add vertices
        if(currentRenderType == ShapeRenderType::FILL){
            // Two triangles to fill
            new_command_if_overflowing(QUAD_INDICES.size());
            auto& points = m_drawCommands.back().points;

            for(int index : QUAD_INDICES){
                points.push_back({QUAD_VERTICES[index] * size + position, currentColor});
            }
        } else {
            // Lines
            new_command_if_overflowing(QUAD_VERTICES.size() * 2);
            auto& points = m_drawCommands.back().points;

            for(size_t i = 0; i < QUAD_VERTICES.size(); i++){
                points.push_back({QUAD_VERTICES[i] * size + position, currentColor});
                points.push_back({QUAD_VERTICES[(i + 1) % QUAD_VERTICES.size()] * size + position, currentColor});
            }
        }
    }

    void ShapeCollection::draw_triangle(const Vector2f& position, const Vector2f& size, float rotation){
        // Profiling
        ZoneScopedN("shape_batch_triangle");

        if(m_commandDirty)
            new_command();

        // Connect all indices, depending on filled or lines
        if(currentRenderType == ShapeRenderType::FILL){
            // One triangles to fill
            new_command_if_overflowing(TRI_VERTICES.size());
            auto& points = m_drawCommands.back().points;

            for(const Vector2f& v : TRI_VERTICES){
                points.push_back({Math::rotate(v * size, rotation) + position, currentColor});
            }
        } else {
            // Lines
            new_command_if_overflowing(TRI_VERTICES.size() * 2);
            auto& points = m_drawCommands.back().points;

            for(size_t i = 0; i < TRI_VERTICES.size(); i++){
                points.push_back({Math::rotate(TRI_VERTICES[i] * size, rotation) + position, currentColor});
                points.push_back({Math::rotate(TRI_VERTICES[(i + 1) % TRI_VERTICES.size()] * size, rotation) + position, currentColor});
            }
        }
    }

    void ShapeCollection::draw_triangle(const std::array<Vector2f, 3>& positions){
        // Profiling
        ZoneScopedN("shape_batch_triangle");

        if(m_commandDirty)
            new_command();

        // Connect all indices, depending on filled or lines
        if(currentRenderType == ShapeRenderType::FILL){
            // Two triangles to fill
            new_command_if_overflowing(positions.size());
            auto& points = m_drawCommands.back().points;

            points.push_back({ positions[0], currentColor });
            points.push_back({ positions[1], currentColor });
            points.push_back({ positions[2], currentColor });
        } else {
            // Lines
            new_command_if_overflowing(positions.size() * 2);
            auto& points = m_drawCommands.back().points;

            for(size_t i = 0; i < positions.size(); i++){
                points.push_back({ positions[i], currentColor });
                points.push_back({ positions[(i + 1) % positions.size()], currentColor });
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
        if(currentRenderType == ShapeRenderType::FILL){
            new_command_if_overflowing(segments * 3);
            auto& points = m_drawCommands.back().points;

            for(size_t i = 0; i < segments; i++){
                float radians1 = i * pointsEveryRadian;
                float radians2 = (i + 1) % segments * pointsEveryRadian;
                Vector2f coords1 = Vector2f(std::cos(radians1 + rotation), std::sin(radians1 + rotation)) * radius + position;
                Vector2f coords2 = Vector2f(std::cos(radians2 + rotation), std::sin(radians2 + rotation)) * radius + position;
                points.push_back({coords1, currentColor});
                points.push_back({coords2, currentColor});
                points.push_back({position, currentColor});
            }
        } else {
            // Lines
            new_command_if_overflowing(segments * 2 + 2);
            auto& points = m_drawCommands.back().points;

            for(size_t i = 0; i < segments; i++){
                float radians1 = i * pointsEveryRadian;
                float radians2 = (i + 1) % segments * pointsEveryRadian;
                Vector2f coords1 = Vector2f(std::cos(radians1 + rotation), std::sin(radians1 + rotation)) * radius + position;
                Vector2f coords2 = Vector2f(std::cos(radians2 + rotation), std::sin(radians2 + rotation)) * radius + position;
                points.push_back({coords1, currentColor});
                points.push_back({coords2, currentColor});
            }

            points.push_back({position, currentColor});
            points.push_back({Vector2f(std::cos(rotation), std::sin(rotation)) * radius + position, currentColor});
        }
    }

    void ShapeCollection::draw_line(const Vector2f& start, const Vector2f& end){
        // Profiling
        ZoneScopedN("shape_batch_line");

        // Lines can only be GL_LINES
        if(currentRenderType != ShapeRenderType::LINE){
            Logger::println(Level::WARNING, "Shape Batch", "draw_line(const Vector2f&, const Vector2f&) may only be called with LINE render type.\n\tIt was set automatically, but you should do it manually.");
            set_render_type(ShapeRenderType::LINE);
        }

        if(m_commandDirty)
            new_command();

        new_command_if_overflowing(2);
        auto& points = m_drawCommands.back().points;

        // Generate and add vertices
        points.push_back({ start, currentColor });
        points.push_back({ end, currentColor });
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
        if(currentRenderType == ShapeRenderType::FILL){
            // Two triangles to fill
            new_command_if_overflowing(6);
            auto& points = m_drawCommands.back().points;

            points.push_back({ start - right, currentColor }); // 1
            points.push_back({ start + right, currentColor }); // 0
            points.push_back({ end + right, currentColor }); // 3
            points.push_back({ start - right, currentColor }); // 1
            points.push_back({ end + right, currentColor }); // 3
            points.push_back({ end - right, currentColor }); // 2
        } else {
            // Lines
            new_command_if_overflowing(8);
            auto& points = m_drawCommands.back().points;

            points.push_back({ start + right, currentColor }); // 0
            points.push_back({ start - right, currentColor }); // 1
            points.push_back({ start - right, currentColor }); // 1
            points.push_back({ end - right, currentColor }); // 2
            points.push_back({ end - right, currentColor }); // 2
            points.push_back({ end + right, currentColor }); // 3
            points.push_back({ end + right, currentColor }); // 3
            points.push_back({ start + right, currentColor }); // 0
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
        if(currentRenderType != ShapeRenderType::LINE){
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
        new_command_if_overflowing(6);
        auto& points = m_drawCommands.back().points;

        points.push_back({ head, currentColor }); // 0
        points.push_back({ head + Vector2f(left.x, left.y), currentColor }); // 1
        points.push_back({ head, currentColor }); // 0
        points.push_back({ head + Vector2f(right.x, right.y), currentColor }); // 2
        points.push_back({ head, currentColor }); // 0
        points.push_back({ tail, currentColor }); // 3
    }

    void ShapeCollection::flush(){
        // Profiling
        ZoneScopedN("shape_batch_flush");

        // Attach the VAO for rendering
        vertexArray.bind();

        while(!m_drawCommands.empty()){
            ShapeDrawCommand& command = m_drawCommands.front();
            auto& points = command.points;

            // Draws all the shapes to opengl
            if(points.empty()){
                m_drawCommands.pop();
                continue;
            }

            // Set shader state
            command.shader->bind();
            command.shader->set_uniform("view", command.transformMatrix);
            command.shader->set_uniform("projection", command.projectionMatrix);
            command.shader->set_uniform("zLayer", command.zLayer);
            
            // Render VBO
            size_t pointsRendered = std::min(points.size(), MAX_POINTS_PER_PASS);

            vertexArray.set_data(0, points);
            glDrawArrays((command.type == ShapeRenderType::LINE) ? GL_LINES : GL_TRIANGLES, 0, pointsRendered);

            m_drawCommands.pop();
        }

        vertexArray.unbind();
        new_command();
    }
};