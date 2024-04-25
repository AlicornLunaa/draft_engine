#include "draft/math/glm.hpp"
#include "draft/rendering/sprite_batch.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/rendering/vertex_buffer.hpp"
#include "glad/gl.h"

#include <cstddef>

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
        vertexID = vertexBuffer.start_buffer<Vertex>(maxSprites * 4); // Each sprite needs 4 vertices
        vertexBuffer.set_attribute(0, GL_FLOAT, 3, sizeof(Vertex), offsetof(Vertex, position));
        vertexBuffer.set_attribute(1, GL_FLOAT, 2, sizeof(Vertex), offsetof(Vertex, texCoords));
        vertexBuffer.end_buffer();

        indicesID = vertexBuffer.start_buffer<int>(maxSprites * 6, GL_ELEMENT_ARRAY_BUFFER); // Each sprite needs 6 indices
        vertexBuffer.set_attribute(2, GL_INT, 1, sizeof(int), 0);
        vertexBuffer.end_buffer();
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

    void SpriteBatch::flush(){
        // Draws all the shapes to opengl
        Texture const* oldTexture = nullptr; // If texture changes, we have to render immediately.
        vector<Vertex> vertices{};
        vector<int> indices{};
        bool flushAgain = false; // Turns true if texture was changed and flush must happen again
        int spriteCount = 0; // Current sprites rendered, stop at max

        // Create vertex geometry
        while(!quadQueue.empty()){
            auto& quad = quadQueue.front();
            auto trans = generate_transform_matrix(quad);
            auto startIndex = vertices.size(); // Used for adding triangle indices

            // Check texture to see if its different
            if(oldTexture == nullptr){
                // First texture, save this pointer
                oldTexture = quad.texture;
            } else if(oldTexture != quad.texture){
                // The texture is different, quit the loop and render everything we have right now
                flushAgain = true;
                break;
            }

            // Vertices
            for(const auto& v : baseVertices){
                // Adds the transformed vertex to the array
                auto pos = trans * Vector3f(v.x, v.y, 1.f);
                pos.z = 0;
                vertices.push_back({pos, {0, 0}});
            }

            // Add texture coordinates based on the floatrect region
            if(quad.region.width <= 0 || quad.region.height <= 0){
                // Less than or equal to zero means the whole texture
                vertices[vertices.size() - 4].texCoords = { 0.f, 0.f };
                vertices[vertices.size() - 3].texCoords = { 1.f, 0.f };
                vertices[vertices.size() - 2].texCoords = { 1.f, 1.f };
                vertices[vertices.size() - 1].texCoords = { 0.f, 1.f };
            } else {
                // Use the float rect region
                Vector2f size = quad.texture->get_size();
                auto& region = quad.region;
                vertices[vertices.size() - 4].texCoords = { region.x / size.x, region.y / size.y };
                vertices[vertices.size() - 3].texCoords = { (region.x + region.width) / size.x, region.y / size.y };
                vertices[vertices.size() - 2].texCoords = { (region.x + region.width) / size.x, (region.y + region.height) / size.y };
                vertices[vertices.size() - 1].texCoords = { region.x / size.x, (region.y + region.height) / size.y };
            }

            // Add indices
            indices.push_back(startIndex + 1);
            indices.push_back(startIndex + 0);
            indices.push_back(startIndex + 3);
            indices.push_back(startIndex + 1);
            indices.push_back(startIndex + 3);
            indices.push_back(startIndex + 2);

            // Remove the quad because its data is stored in the vertices now
            quadQueue.pop();

            // Check sprite count
            if(++spriteCount >= maxSprites){
                flushAgain = true;
                break;
            }
        }

        // Early exit if theres nothing to do
        if(!oldTexture || vertices.size() <= 0)
            return;

        // Create vertex buffer and render it
        vertexBuffer.set_dynamic_data(vertexID, vertices);
        vertexBuffer.set_dynamic_data(indicesID, indices);

        vertexBuffer.bind();
        oldTexture->bind();
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        vertexBuffer.unbind();

        // Do it again for the rest of the quads
        if(flushAgain)
            flush();
    }
};