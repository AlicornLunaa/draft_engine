#include "draft/math/glm.hpp"
#include "draft/rendering/batching/shape_batch.hpp"
#include "draft/rendering/batching/batch.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/vertex_buffer.hpp"
#include "draft/util/logger.hpp"
#include "glad/gl.h"

using namespace std;

namespace Draft {
    // Constructor
    ShapeBatch::ShapeBatch(std::shared_ptr<Shader> shader) : Batch(shader) {
        // Setup data buffers
        dynamicVertexBufLoc = vertexBuffer.start_buffer<Point>(MAX_SHAPES_TO_RENDER);
        vertexBuffer.set_attribute(0, GL_FLOAT, 2, sizeof(Point), 0);
        vertexBuffer.set_attribute(1, GL_FLOAT, 4, sizeof(Point), offsetof(Point, color));
        vertexBuffer.end_buffer();
    }

    // Functions
    void ShapeBatch::set_render_type(RenderType type){
        if(currentRenderType != type)
            flush();

        currentRenderType = type;
    }

    void ShapeBatch::draw_polygon(const std::vector<Vector2f>& polygonVertices){
        // Generate and add vertices
        for(size_t i = 0; i < polygonVertices.size(); i++){
            points.push_back({ polygonVertices[i], currentColor });
            points.push_back({ polygonVertices[(i + 1) % polygonVertices.size()], currentColor });
        }
    }

    void ShapeBatch::draw_rect(const Vector2f& position, const Vector2f& size, float rotation){
        // Generate and add vertices
        if(currentRenderType == RenderType::FILL){
            // Two triangles to fill
            for(int index : QUAD_INDICES){
                points.push_back({QUAD_VERTICES[index] * size + position, currentColor});
            }
        } else {
            // Lines
            for(size_t i = 0; i < QUAD_VERTICES.size(); i++){
                points.push_back({QUAD_VERTICES[i] * size + position, currentColor});
                points.push_back({QUAD_VERTICES[(i + 1) % QUAD_VERTICES.size()] * size + position, currentColor});
            }
        }
    }

    void ShapeBatch::draw_triangle(const Vector2f& position, const Vector2f& size, float rotation){
        // Connect all indices, depending on filled or lines
        if(currentRenderType == RenderType::FILL){
            // Two triangles to fill
            for(const Vector2f& v : TRI_VERTICES){
                points.push_back({Math::rotate(v * size, rotation) + position, currentColor});
            }
        } else {
            // Lines
            for(size_t i = 0; i < TRI_VERTICES.size(); i++){
                points.push_back({Math::rotate(TRI_VERTICES[i] * size, rotation) + position, currentColor});
                points.push_back({Math::rotate(TRI_VERTICES[(i + 1) % TRI_VERTICES.size()] * size, rotation) + position, currentColor});
            }
        }
    }

    void ShapeBatch::draw_triangle(const std::array<Vector2f, 3>& positions){
        // Generate and add vertices

        // Connect all indices, depending on filled or lines
        if(currentRenderType == RenderType::FILL){
            // Two triangles to fill
            points.push_back({ positions[0], currentColor });
            points.push_back({ positions[1], currentColor });
            points.push_back({ positions[2], currentColor });
        } else {
            // Lines
            for(size_t i = 0; i < positions.size(); i++){
                points.push_back({ positions[i], currentColor });
                points.push_back({ positions[(i + 1) % positions.size()], currentColor });
            }
        }
    }

    void ShapeBatch::draw_circle(const Vector2f& position, float radius, float rotation, size_t segments){
        // Generate and add vertices
        float pointsEveryRadian = 2*3.14f / segments;

        // Connect all indices, depending on filled or lines
        if(currentRenderType == RenderType::FILL){
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

    void ShapeBatch::draw_line(const Vector2f& start, const Vector2f& end){
        // Lines can only be GL_LINES
        if(currentRenderType != RenderType::LINE){
            Logger::println(Level::WARNING, "Shape Batch", "draw_line(const Vector2f&, const Vector2f&) may only be called with LINE render type.\n\tIt was set automatically, but you should do it manually.");
            set_render_type(RenderType::LINE);
        }

        // Generate and add vertices
        points.push_back({ start, currentColor });
        points.push_back({ end, currentColor });
    }

    void ShapeBatch::draw_rect_line(const Vector2f& start, const Vector2f& end, float width){
        // Generate and add vertices
        float radians = std::atan2(start.y - end.y, start.x - end.x);
        Vector2f right = Math::rotate(Vector2f(0, 1), radians);
        right *= 0.02f * width;

        points.push_back({ start + right, currentColor }); // 0
        points.push_back({ start - right, currentColor }); // 1
        points.push_back({ end - right, currentColor }); // 2
        points.push_back({ end + right, currentColor }); // 3

        // Connect all indices, depending on filled or lines
        if(currentRenderType == RenderType::FILL){
            // Two triangles to fill
            points.push_back({ start - right, currentColor }); // 1
            points.push_back({ start + right, currentColor }); // 0
            points.push_back({ end + right, currentColor }); // 3
            points.push_back({ start - right, currentColor }); // 1
            points.push_back({ end + right, currentColor }); // 3
            points.push_back({ end - right, currentColor }); // 2
        } else {
            // Lines
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

    void ShapeBatch::draw_arrow(const Vector2f& head, const Vector2f& tail){
        // Lines can only be GL_LINES
        if(currentRenderType != RenderType::LINE){
            Logger::println(Level::WARNING, "Shape Batch", "draw_arrow(const Vector2f&, const Vector2f&) may only be called with LINE render type.\n\tIt was set automatically, but you should do it manually.");
            set_render_type(RenderType::LINE);
        }

        // Get the size vertices for the arrow head
        float radians = std::atan2(head.y - tail.y, head.x - tail.x);
        Vector2f left = Math::rotate(Vector2f(0, 1), radians + 3.141592654f/3.f) * 0.08f;
        Vector2f right = Math::rotate(Vector2f(0, 1), radians + 2*3.141592654f/3.f) * 0.08f;

        // Generate and add vertices
        points.push_back({ head, currentColor }); // 0
        points.push_back({ head + Vector2f(left.x, left.y), currentColor }); // 1
        points.push_back({ head, currentColor }); // 0
        points.push_back({ head + Vector2f(right.x, right.y), currentColor }); // 2
        points.push_back({ head, currentColor }); // 0
        points.push_back({ tail, currentColor }); // 3
    }

    void ShapeBatch::begin(){
        Batch::begin();
        points.clear();
    }

    void ShapeBatch::flush(){
        // Draws all the shapes to opengl
        if(points.empty())
            return;

        // Render VBO
        shaderPtr->bind();
        shaderPtr->set_uniform("view", get_trans_matrix());
        shaderPtr->set_uniform("projection", get_proj_matrix());
        
        for(size_t i = 0; i <= points.size() / MAX_SHAPES_TO_RENDER; i++){
            // Repeat for number of render chunks
            size_t pointsRendered = std::min(points.size(), MAX_SHAPES_TO_RENDER);

            vertexBuffer.bind();
            vertexBuffer.set_dynamic_data(dynamicVertexBufLoc, points);

            glDrawArrays((currentRenderType == RenderType::LINE) ? GL_LINES : GL_TRIANGLES, 0, pointsRendered);

            points.erase(points.begin(), points.begin() + pointsRendered);
        }

        vertexBuffer.unbind();
    }

    void ShapeBatch::end(){
        Batch::end();
    }
};