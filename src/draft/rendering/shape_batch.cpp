#include "draft/math/glm.hpp"
#include "draft/rendering/shape_batch.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/vertex_buffer.hpp"
#include "draft/util/logger.hpp"
#include "glad/gl.h"

using namespace std;

namespace Draft {
    // Static data
    array<Vector2f, 4> ShapeBatch::quadVertices = array<Vector2f, 4>({
        {0.f, 0.f}, // Top left
        {1.f, 0.f}, // Top right
        {1.f, 1.f}, // Bottom right
        {0.f, 1.f}  // Bottom left
    });

    // Private function
    std::tuple<ShapeBatch::RenderType, size_t, size_t>& ShapeBatch::get_current_render_type_instance(){
        if(!renderTypes.empty())
            return renderTypes.back();

        renderTypes.push({ currentRenderType, 0, 0 });
        return renderTypes.back();
    }

    // Constructor
    ShapeBatch::ShapeBatch(std::shared_ptr<Shader> shader, const size_t maxShapes) : maxShapes(maxShapes), shader(shader) {
        // Setup data buffers
        dynamicVertexBufLoc = vertexBuffer.start_buffer<ShapeVertex>(maxShapes);
        vertexBuffer.set_attribute(0, GL_FLOAT, 2, sizeof(ShapeVertex), 0);
        vertexBuffer.set_attribute(1, GL_FLOAT, 4, sizeof(ShapeVertex), offsetof(ShapeVertex, color));
        vertexBuffer.end_buffer();

        dynamicIndexBufLoc = vertexBuffer.start_buffer<int>(maxShapes * 2, GL_ELEMENT_ARRAY_BUFFER);
        vertexBuffer.end_buffer();
    }

    // Functions
    void ShapeBatch::set_render_type(RenderType type){
        if(get<0>(get_current_render_type_instance()) == type)
            return; // Skip if its the same type

        currentRenderType = type;
        renderTypes.push({ type, 0, 0 });
    }

    void ShapeBatch::draw_polygon(const std::vector<Vector2f>& polygonVertices){
        // Lines can only be GL_LINES
        if(currentRenderType != RenderType::LINE){
            Logger::println(Level::WARNING, "Shape Batch", "draw_polygon(const std::vector<Vector2f>&) may only be called with LINE render type.\n\tIt was set automatically, but you should do it manually.");
            set_render_type(RenderType::LINE);
        }

        // Generate and add vertices
        auto& tup = get_current_render_type_instance();
        size_t indexStart = vertices.size();

        for(auto& v : polygonVertices){
            vertices.push_back({ v, currentColor });
        }
        get<1>(tup) += polygonVertices.size();

        // Connect all indices
        for(size_t i = 0; i < polygonVertices.size(); i++){
            indices.push_back(i + indexStart);
            indices.push_back((i + 1) % polygonVertices.size() + indexStart);
        }
        get<2>(tup) += (polygonVertices.size() * 2);
    }

    void ShapeBatch::draw_rect(const Vector2f& position, const Vector2f& size, float rotation){
        // Generate and add vertices
        auto& tup = get_current_render_type_instance();
        size_t indexStart = vertices.size();

        vertices.push_back({ position, currentColor });
        vertices.push_back({ position + Vector2f{ size.x, 0.f }, currentColor });
        vertices.push_back({ position + Vector2f{ size.x, size.y }, currentColor });
        vertices.push_back({ position + Vector2f{ 0.f, size.y }, currentColor });
        get<1>(tup) += 4;

        // Connect all indices, depending on filled or lines
        if(currentRenderType == RenderType::FILL){
            // Two triangles to fill
            indices.push_back(1 + indexStart);
            indices.push_back(0 + indexStart);
            indices.push_back(3 + indexStart);
            indices.push_back(1 + indexStart);
            indices.push_back(3 + indexStart);
            indices.push_back(2 + indexStart);
            get<2>(tup) += 6;
        } else {
            // Lines
            for(size_t i = 0; i < 4; i++){
                indices.push_back(i + indexStart);
                indices.push_back((i + 1) % 4 + indexStart);
            }
            
            // Increase length for render type
            get<2>(tup) += 8;
        }
    }

    void ShapeBatch::draw_circle(const Vector2f& position, float radius, float rotation, size_t segments){
        // Generate and add vertices
        auto& tup = get_current_render_type_instance();
        size_t indexStart = vertices.size();
        float pointsEveryRadian = 2*3.14f / segments;

        vertices.push_back({ position, currentColor });

        // Circular vertices
        for(size_t i = 0; i < segments; i++){
            float radians = i * pointsEveryRadian;
            Vector2f coords(std::cos(radians + rotation), std::sin(radians + rotation));
            coords *= radius;
            coords += position;
            vertices.push_back({coords, currentColor});
        }
        get<1>(tup) += (segments + 1);

        // Connect all indices, depending on filled or lines
        if(currentRenderType == RenderType::FILL){
            // Triangles, so 0, i, i + 1
            for(size_t i = 0; i < segments; i++){
                indices.push_back(indexStart);
                indices.push_back(i % segments + indexStart + 1);
                indices.push_back((i + 1) % segments + indexStart + 1);
            }
            
            // Increase length for render type
            get<2>(tup) += (segments * 3);
        } else {
            // Lines
            for(size_t i = 0; i < segments; i++){
                indices.push_back(indexStart + i);
                indices.push_back((i + 1) % (segments + 1) + indexStart);
            }
            indices.push_back(indexStart + segments);
            indices.push_back(indexStart + 1);
            
            // Increase length for render type
            get<2>(tup) += (segments * 2 + 2);
        }
    }

    void ShapeBatch::draw_line(const Vector2f& start, const Vector2f& end){
        // Lines can only be GL_LINES
        if(currentRenderType != RenderType::LINE){
            Logger::println(Level::WARNING, "Shape Batch", "draw_line(const Vector2f&, const Vector2f&) may only be called with LINE render type.\n\tIt was set automatically, but you should do it manually.");
            set_render_type(RenderType::LINE);
        }

        // Generate and add vertices
        size_t indexStart = vertices.size();
        vertices.push_back({ start, currentColor });
        vertices.push_back({ end, currentColor });
        indices.push_back(indexStart);
        indices.push_back(indexStart + 1);
        
        // Increase length for render type
        auto& tup = get_current_render_type_instance();
        get<1>(tup) += 2;
        get<2>(tup) += 2;
    }

    void ShapeBatch::draw_rect_line(const Vector2f& start, const Vector2f& end, float width){
        // Generate and add vertices
        auto& tup = get_current_render_type_instance();
        size_t indexStart = vertices.size();

        float radians = std::atan2(start.y - end.y, start.x - end.x);
        Vector2f right = Math::rotate(Vector2f(0, 1), radians);
        right *= 0.02f * width;

        vertices.push_back({ start + right, currentColor });
        vertices.push_back({ start - right, currentColor });
        vertices.push_back({ end - right, currentColor });
        vertices.push_back({ end + right, currentColor });
        get<1>(tup) += 4;

        // Connect all indices, depending on filled or lines
        if(currentRenderType == RenderType::FILL){
            // Two triangles to fill
            indices.push_back(1 + indexStart);
            indices.push_back(0 + indexStart);
            indices.push_back(3 + indexStart);
            indices.push_back(1 + indexStart);
            indices.push_back(3 + indexStart);
            indices.push_back(2 + indexStart);
            get<2>(tup) += 6;
        } else {
            // Lines
            for(size_t i = 0; i < 4; i++){
                indices.push_back(i + indexStart);
                indices.push_back((i + 1) % 4 + indexStart);
            }
            
            // Increase length for render type
            get<2>(tup) += 8;
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
        size_t indexStart = vertices.size();
        vertices.push_back({ head, currentColor });
        vertices.push_back({ head + Vector2f(left.x, left.y), currentColor });
        vertices.push_back({ head + Vector2f(right.x, right.y), currentColor });
        vertices.push_back({ tail, currentColor });
        indices.push_back(indexStart);
        indices.push_back(indexStart + 1);
        indices.push_back(indexStart);
        indices.push_back(indexStart + 2);
        indices.push_back(indexStart);
        indices.push_back(indexStart + 3);
        
        // Increase length for render type
        auto& tup = get_current_render_type_instance();
        get<1>(tup) += 4;
        get<2>(tup) += 6;
    }

    void ShapeBatch::flush(const RenderWindow& window, const Camera* camera){
        // Draws all the shapes to opengl
        bool flushAgain = false; // Turns true if the shape type changed

        // Early exit if theres nothing to do
        if(vertices.empty() || indices.empty() || renderTypes.empty())
            return;

        // Run through for each draw type
        auto [type, vertexCount, indexCount] = renderTypes.front();
        renderTypes.pop();

        // Check if this has zero data
        if(vertexCount == 0 || indexCount == 0){
            flush(window, camera);
            return;
        }

        // Check if this run needs to be chopped up
        if(vertexCount > maxShapes || indexCount > maxShapes * 2){
            // Buffer has to be run in two parts
            renderTypes.emplace(type, vertexCount - maxShapes, indexCount - maxShapes * 2);
            vertexCount = maxShapes;
            indexCount = maxShapes * 2;
            flushAgain = true;
        }

        // Buffer data so far
        vertexBuffer.set_dynamic_data(dynamicVertexBufLoc, vertices);
        vertexBuffer.set_dynamic_data(dynamicIndexBufLoc, indices);

        // Render VBO
        shader->bind();
        
        if(camera)
            camera->apply(window, shader);

        vertexBuffer.bind();
        glDrawElements((type == RenderType::LINE) ? GL_LINES : GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        vertexBuffer.unbind();

        // Clean used vertices
        if(renderTypes.empty()){
            vertices.clear();
            indices.clear();
        } else {
            vertices.erase(vertices.begin(), vertices.begin() + vertexCount);
            indices.erase(indices.begin(), indices.begin() + indexCount);

            // Shift indices for the deleted vertices
            for(auto& index : indices){
                index -= vertexCount;
            }

            flushAgain = true;
        }

        // Do it again for the rest of the quads
        if(flushAgain)
            flush(window, camera);
    }
};