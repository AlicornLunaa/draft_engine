#include "draft/rendering/sprite_batch.hpp"
#include "draft/math/matrix.hpp"
#include "draft/math/vector3.hpp"
#include "draft/math/vector4.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/rendering/vertex_buffer.hpp"
#include "glad/gl.h"

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
    Matrix4 SpriteBatch::generate_transform_matrix(const Quad& quad) const {
        // Generates a transformation matrix for the given quad
        return Matrix4::translation({ quad.position.x, quad.position.y, 0.f }) * Matrix4::scale({ quad.size.x, quad.size.y, 1.f }) * Matrix4::rotation({ 0.f, 0.f, quad.rotation });
    }

    // Constructor
    SpriteBatch::SpriteBatch(){}

    // Functions
    void SpriteBatch::draw(const Texture& texture, const Vector2f& position, const Vector2f& size, float rotation, FloatRect region){
        // Add quad to the queue
        Quad quad {
            &texture,
            region,

            position,
            size,
            rotation
        };

        quadQueue.emplace(quad);
    }

    void SpriteBatch::flush(){
        // Draws all the shapes to opengl
        Texture const * oldTexture = nullptr; // If texture changes, we have to render immediately.
        vector<Vector3f> vertices;
        vector<Vector2f> texCoords;
        vector<int> indices;
        bool flushAgain = false; // Turns true if texture was changed and flush must happen again

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
                auto transformedV = trans * Vector4f(v.x, v.y, 0, 1);
                vertices.push_back({ transformedV.x, transformedV.y, 0.f });
            }

            // Add texture coordinates based on the floatrect region
            if(quad.region.width <= 0 || quad.region.height <= 0){
                // Less than or equal to zero means the whole texture
                Vector2f size = quad.texture->get_size();
                texCoords.push_back({ 0.f, 0.f });
                texCoords.push_back({ 1.f, 0.f });
                texCoords.push_back({ 1.f, 1.f });
                texCoords.push_back({ 0.f, 1.f });
            } else {
                // Use the float rect region
                Vector2f size = quad.texture->get_size();
                auto& region = quad.region;
                texCoords.push_back({ region.x / size.x, region.y / size.y });
                texCoords.push_back({ (region.x + region.width) / size.x, region.y / size.y });
                texCoords.push_back({ (region.x + region.width) / size.x, (region.y + region.height) / size.y });
                texCoords.push_back({ region.x / size.x, (region.y + region.height) / size.y });
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
        }

        // Early exit if theres nothing to do
        if(!oldTexture || vertices.size() <= 0)
            return;

        // Create vertex buffer and render it
        VertexBuffer vbo;
        vbo.buffer(0, vertices);
        vbo.buffer(1, texCoords);
        vbo.buffer(2, indices, GL_ELEMENT_ARRAY_BUFFER);

        oldTexture->bind();
        vbo.bind();
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        vbo.unbind();

        // Do it again for the rest of the quads
        if(flushAgain)
            flush();
    }
};