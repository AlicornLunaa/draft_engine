#include "draft/math/glm.hpp"
#include "draft/rendering/sprite_batch.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/rendering/vertex_buffer.hpp"
#include "glad/gl.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

using namespace std;

namespace Draft {
    // Constructor
    SpriteBatch::SpriteBatch(const Shader& shader, const size_t maxSprites) : maxSprites(maxSprites), shader(shader) {
        // Buffer the data on the GPU
        dynamicVertexBufLoc = vertexBuffer.start_buffer<QuadVertex>(maxSprites * 4);
        vertexBuffer.set_attribute(0, GL_FLOAT, 2, sizeof(QuadVertex), 0);
        vertexBuffer.set_attribute(1, GL_FLOAT, 2, sizeof(QuadVertex), offsetof(QuadVertex, texCoords));
        vertexBuffer.set_attribute(2, GL_FLOAT, 4, sizeof(QuadVertex), offsetof(QuadVertex, color));
        vertexBuffer.end_buffer();

        dynamicIndexBufLoc = vertexBuffer.start_buffer<int>(maxSprites * 6, GL_ELEMENT_ARRAY_BUFFER);
        vertexBuffer.end_buffer();

        vertices.reserve(maxSprites * 4);
        indices.reserve(maxSprites * 6);
    }

    // Functions
    void SpriteBatch::set_color(const Vector4f& color){ currentColor = color; }
    const Vector4f& SpriteBatch::get_color() const { return currentColor; }

    void SpriteBatch::draw(const Texture& texture, const Vector2f& position, const Vector2f& size, float rotation, const Vector2f& origin, FloatRect region){
        // Add quad to the queue
        size_t index = vertices.size();

        // // Obtain texture ID
        if(!textureRegister.empty() && textureRegister.back().first == &texture){
            // Current texture is the same texture in the queue, increase the quad count
            textureRegister.back().second++;
        } else {
            // Current texture is different, add new texture
            textureRegister.push({&texture, 1});
        }

        // Create texture coord data
        auto& textureSize = texture.get_size();
        float x = region.x / textureSize.x;
        float y = region.y / textureSize.y;
        float w = ((region.x <= 0) ? texture.get_size().x : region.x) / textureSize.x;
        float h = ((region.y <= 0) ? texture.get_size().y : region.y) / textureSize.y;

        // Create vertices, TODO: Texture coords based on region
        vertices.push_back({ {0, 0}, {x, y}, currentColor });
        vertices.push_back({ {1, 0}, {x + w, y}, currentColor });
        vertices.push_back({ {1, 1}, {x + w, y + h}, currentColor });
        vertices.push_back({ {0, 1}, {x, y + h}, currentColor });

        // Rotate quad positions if needed
        auto rotate_vertex = [position, rotation, size, origin](Vector2f& v){
            float s = std::sin(rotation);
            float c = std::cos(rotation);
            Matrix2 mat(c, s, -s, c);

            v *= size;
            v -= origin;
            v = mat * v;
            v += position;
        };
        rotate_vertex(vertices[index].position);
        rotate_vertex(vertices[index + 1].position);
        rotate_vertex(vertices[index + 2].position);
        rotate_vertex(vertices[index + 3].position);

        // // Create indices
        indices.push_back(1 + index);
        indices.push_back(0 + index);
        indices.push_back(3 + index);
        indices.push_back(1 + index);
        indices.push_back(3 + index);
        indices.push_back(2 + index);
    }

    void SpriteBatch::flush(){
        // Draws all the shapes to opengl
        bool flushAgain = false; // Turns true if texture was changed and flush must happen again

        // Exit early if theres nothing to do
        if(vertices.empty() || indices.empty())
            return;

        shader.bind();

        // Render each texture
        while(!textureRegister.empty()){
            // Get data
            auto [texture, quadCount] = textureRegister.front();
            size_t maxQuads = std::min(quadCount, maxSprites);

            // Buffer data
            vertexBuffer.set_dynamic_data(dynamicVertexBufLoc, vertices);
            vertexBuffer.set_dynamic_data(dynamicIndexBufLoc, indices);

            // Render all the vertices
            texture->bind();
            vertexBuffer.bind();
            glDrawElements(GL_TRIANGLES, maxQuads * 6, GL_UNSIGNED_INT, 0);
            vertexBuffer.unbind();

            // Cleanup used vertices and prep for next iteration for textures
            vertices.erase(vertices.begin(), vertices.begin() + maxQuads * 4);
            indices.erase(indices.begin(), indices.begin() + maxQuads * 6);

            // Shift indices for the deleted vertices
            for(auto& index : indices){
                index -= maxQuads * 4;
            }

            // Check if the vertices were completed
            if(quadCount > maxSprites){
                // Run again
                textureRegister.front().second -= maxQuads;
            } else {
                // Remove this iteration
                textureRegister.pop();
            }
        }

        // Clear out data for next flush
        vertices.clear();
        indices.clear();

        // Do it again for the rest of the quads
        if(flushAgain)
            flush();
    }
};