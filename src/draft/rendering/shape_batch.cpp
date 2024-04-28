#include "draft/math/glm.hpp"
#include "draft/rendering/shape_batch.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/vertex_buffer.hpp"
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
    std::tuple<ShapeBatch::RenderType, size_t, size_t>& ShapeBatch::get_current_drawtype_instance(){
        if(drawTypes.size() > 0)
            return drawTypes.back();

        drawTypes.push_back({ currentRenderType, 0, 0 });
        return drawTypes.back();
    }

    // Constructor
    ShapeBatch::ShapeBatch(Shader& shader, const size_t maxShapes) : maxShapes(maxShapes), shader(shader) {
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
        if(get<0>(get_current_drawtype_instance()) == type)
            return; // Skip if its the same type

        drawTypes.push_back({ type, 0, 0 });
    }

    void ShapeBatch::draw_circle(const Vector2f& position, float radius, float rotation, size_t segments){
        // Generate and add vertices
        size_t indexStart = vertices.size();
        float pointsEveryRadian = 2*3.14f / segments;

        vertices.push_back({ position, currentColor });

        // Circular vertices
        for(float i = 0; i < 2*3.14f; i += pointsEveryRadian){
            Vector2f coords(std::cos(i + rotation), std::sin(i + rotation));
            coords *= radius;
            coords += position;
            vertices.push_back({coords, currentColor});
        }

        // Connect all indices
        for(size_t i = 0; i < segments; i++){
            indices.push_back(indexStart + i);
            indices.push_back((i + 1) % (segments + 1) + indexStart);
        }
        indices.push_back(indexStart + segments);
        indices.push_back(indexStart + 1);
        
        // Increase length for render type
        auto& tup = get_current_drawtype_instance();
        get<1>(tup) += (segments + 1);
        get<2>(tup) += (segments * 2 + 2);
    }

    void ShapeBatch::draw_line(const Vector2f& start, const Vector2f& end){
        // Generate and add vertices
        size_t indexStart = vertices.size();
        vertices.push_back({ start, currentColor });
        vertices.push_back({ end, currentColor });
        indices.push_back(indexStart);
        indices.push_back(indexStart + 1);
        
        // Increase length for render type
        auto& tup = get_current_drawtype_instance();
        get<1>(tup) += 2;
        get<2>(tup) += 2;
    }

    void ShapeBatch::flush(){
        // Draws all the shapes to opengl
        bool flushAgain = false; // Turns true if the shape type changed

        // Early exit if theres nothing to do
        if(vertices.size() <= 0)
            return;

        // Run through for each draw type
        auto& [type, vertexCount, indexCount] = get_current_drawtype_instance();
        drawTypes.erase(drawTypes.begin(), drawTypes.begin() + 1);

        // Check if this run needs to be chopped up
        if(vertexCount > maxShapes){
            // Buffer has to be run in two parts
            drawTypes.push_back({ type, vertexCount - maxShapes, indexCount - maxShapes * 2 });
            vertexCount = maxShapes;
            indexCount = maxShapes * 2;
            flushAgain = true;
        }

        // Buffer data so far
        vertexBuffer.set_dynamic_data(dynamicVertexBufLoc, vertices);
        vertexBuffer.set_dynamic_data(dynamicIndexBufLoc, indices);

        // Render VBO
        vertexBuffer.bind();
        glDrawElements((type == RenderType::LINE) ? GL_LINES : GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        vertexBuffer.unbind();

        // Clean used vertices
        if(drawTypes.empty()){
            vertices.clear();
            indices.clear();
        } else {
            vertices.erase(vertices.begin(), vertices.begin() + vertexCount);
            indices.erase(indices.begin(), indices.begin() + indexCount);
            flushAgain = true;
        }

        // Do it again for the rest of the quads
        if(flushAgain)
            flush();
    }
};