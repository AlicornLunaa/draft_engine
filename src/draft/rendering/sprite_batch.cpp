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
    // Static data
    array<Vector2f, 4> SpriteBatch::baseVertices = array<Vector2f, 4>({
        {0.f, 0.f}, // Top left
        {1.f, 0.f}, // Top right
        {1.f, 1.f}, // Bottom right
        {0.f, 1.f}  // Bottom left
    });

    // Private functions
    Matrix3 SpriteBatch::generate_transform_matrix(const Quad& quad) const {
        // Generates a transformation matrix for the given quad
        auto trans = Matrix3(1.f);
        trans = Math::translate(trans, quad.position);
        trans = Math::rotate(trans, quad.rotation);
        trans = Math::translate(trans, -quad.origin);
        trans = Math::scale(trans, quad.size);
        return trans;
    }

    // Constructor
    SpriteBatch::SpriteBatch(const size_t maxSprites) : maxSprites(maxSprites) {
        vertexBuffer.buffer(0, vector<Vector3f>{{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}});
        vertexBuffer.buffer(1, vector<Vector2f>{{0, 0}, {1, 0}, {1, 1}, {0, 1}});
        vertexBuffer.buffer(2, vector<int>{1, 0, 3, 1, 3, 2}, GL_ELEMENT_ARRAY_BUFFER);
    }

    // Functions
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

    void SpriteBatch::flush(Shader& shader){
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
            auto quadStrID = "quads[" + to_string(spriteCount) + "]";
            shader.set_uniform(quadStrID + ".position", quad.position);
            shader.set_uniform(quadStrID + ".rotation", quad.rotation);
            shader.set_uniform(quadStrID + ".origin", quad.origin);
            shader.set_uniform(quadStrID + ".size", quad.size);
            shader.set_uniform(quadStrID + ".texCoord[0]", texCoords[0]);
            shader.set_uniform(quadStrID + ".texCoord[1]", texCoords[1]);
            shader.set_uniform(quadStrID + ".texCoord[2]", texCoords[2]);
            shader.set_uniform(quadStrID + ".texCoord[3]", texCoords[3]);

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
            flush(shader);
    }
};