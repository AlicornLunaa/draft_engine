#include "draft/math/glm.hpp"
#include "draft/rendering/sprite_batch.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/rendering/vertex_buffer.hpp"
#include "glad/gl.h"

#include <cstddef>
#include <string>
#include <vector>

using namespace std;

namespace Draft {
    // Constructor
    SpriteBatch::SpriteBatch(const Shader& shader, const size_t maxSprites) : maxSprites(maxSprites), shader(shader) {
        // Buffer the data on the GPU
        vertexBuffer.buffer(0, vector<Vector3f>{{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}});
        vertexBuffer.buffer(1, vector<Vector2f>{{0, 0}, {1, 0}, {1, 1}, {0, 1}});
        vertexBuffer.buffer(2, vector<int>{1, 0, 3, 1, 3, 2}, GL_ELEMENT_ARRAY_BUFFER);

        // Setup the uniform names
        for(size_t i = 0; i < maxSprites; i++){
            std::string quadStrID = "quads[" + to_string(i) + "]";
            uniformLocations.push_back(shader.get_location(quadStrID + ".position"));
            uniformLocations.push_back(shader.get_location(quadStrID + ".rotation"));
            uniformLocations.push_back(shader.get_location(quadStrID + ".origin"));
            uniformLocations.push_back(shader.get_location(quadStrID + ".size"));
            uniformLocations.push_back(shader.get_location(quadStrID + ".texCoord[0]"));
            uniformLocations.push_back(shader.get_location(quadStrID + ".texCoord[1]"));
            uniformLocations.push_back(shader.get_location(quadStrID + ".texCoord[2]"));
            uniformLocations.push_back(shader.get_location(quadStrID + ".texCoord[3]"));
        }
    }

    // Functions
    void SpriteBatch::set_color(const Vector4f& color){ currentColor = color; }
    const Vector4f& SpriteBatch::get_color() const { return currentColor; }

    void SpriteBatch::draw(const Texture& texture, const Vector2f& position, const Vector2f& size, float rotation, const Vector2f& origin, FloatRect region){
        // Add quad to the queue
        Quad quad {
            &texture,
            region,

            position,
            size,
            rotation,
            origin
        };

        quadQueue.emplace(quad);
    }

    void SpriteBatch::flush(){
        // Draws all the shapes to opengl
        Texture const* oldTexture = nullptr; // If texture changes, we have to render immediately.
        Vector2f texCoords[4]{}; // Temporarily hold the texture coordinates for the shader
        bool flushAgain = false; // Turns true if texture was changed and flush must happen again
        int spriteCount = 0; // Current sprites rendered, stop at max

        shader.bind();

        // Create vertex geometry
        while(!quadQueue.empty()){
            // Get the next quad in the queue
            auto& quad = quadQueue.front();

            // Check texture to see if its different
            if(oldTexture == nullptr){
                // First texture, save this pointer
                oldTexture = quad.texture;
            } else if(oldTexture != quad.texture){
                // The texture is different, quit the loop and render everything we have right now
                flushAgain = true;
                break;
            }

            // Add texture coordinates based on the floatrect region
            if(quad.region.width <= 0 || quad.region.height <= 0){
                // Less than or equal to zero means the whole texture
                texCoords[0].x = 0.f; texCoords[0].y = 0.f;
                texCoords[1].x = 1.f; texCoords[1].y = 0.f;
                texCoords[2].x = 1.f; texCoords[2].y = 1.f;
                texCoords[3].x = 0.f; texCoords[3].y = 1.f;
            } else {
                // Use the float rect region
                Vector2f size = quad.texture->get_size();
                auto& region = quad.region;
                texCoords[0].x = region.x / size.x;                  texCoords[0].y = region.y / size.y;
                texCoords[1].x = (region.x + region.width) / size.x; texCoords[1].y = region.y / size.y;
                texCoords[2].x = (region.x + region.width) / size.x; texCoords[2].y = (region.y + region.height) / size.y;
                texCoords[3].x = region.x / size.x;                  texCoords[3].y = (region.y + region.height) / size.y;
            }

            // Send this quad's data to the shader
            int uniformGroupID = spriteCount * 8;
            shader.set_uniform(uniformLocations[uniformGroupID], quad.position);
            shader.set_uniform(uniformLocations[uniformGroupID + 1], quad.rotation);
            shader.set_uniform(uniformLocations[uniformGroupID + 2], quad.origin);
            shader.set_uniform(uniformLocations[uniformGroupID + 3], quad.size);
            shader.set_uniform(uniformLocations[uniformGroupID + 4], texCoords[0]);
            shader.set_uniform(uniformLocations[uniformGroupID + 5], texCoords[1]);
            shader.set_uniform(uniformLocations[uniformGroupID + 6], texCoords[2]);
            shader.set_uniform(uniformLocations[uniformGroupID + 7], texCoords[3]);

            // Remove the quad because its data is stored in the vertices now
            quadQueue.pop();

            // Check sprite count
            if(++spriteCount >= maxSprites){
                flushAgain = true;
                break;
            }
        }

        // Early exit if theres nothing to do
        if(!oldTexture)
            return;

        // Render it 
        vertexBuffer.bind();
        oldTexture->bind();
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, spriteCount);
        vertexBuffer.unbind();

        // Do it again for the rest of the quads
        if(flushAgain)
            flush();
    }
};